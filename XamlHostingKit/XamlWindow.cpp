#include "pch.h"
#include "XamlWindow.h"
#include "XamlWindow.g.cpp"

#include <dwmapi.h>
#include <shellapi.h>
#include "XamlConfig.h"
#include "LegacyNonImmersiveView.h"
#include <CoreWindow.h>
#include "Features.h"
#include "XamlApplication.h"

namespace winrt::XamlHostingKit::implementation
{
    thread_local XamlWindow* XamlWindow::s_currentWindow = nullptr;

    XamlWindow::XamlWindow(winrt::XamlHostingKit::WindowCreationOptions const& options, bool isMain)
        : m_isMain(isMain)
    {
        static const auto className = RegisterWindowClass(WndProc);

        m_title = options.Title();
        m_styles = options.Styles();
        m_extendedStyles = options.ExtendedStyles();

        BOOL dwmFrameEnabled = TRUE;
        if (FAILED(DwmIsCompositionEnabled(&dwmFrameEnabled))) [[unlikely]]
        {
            dwmFrameEnabled = TRUE;
        }

        //EnableMouseInPointer(TRUE);

        if (XamlConfig::s_enableTouchpadAwareness) [[likely]]
        {
            if (RegisterTouchpadCapableThreadMethod) [[unlikely]]
            {
                LOG_LAST_ERROR_IF(!RegisterTouchpadCapableThreadMethod(TRUE));
            }
            else if (NtUserRegisterTouchPadCapable) [[likely]]
            {
                LOG_LAST_ERROR_IF(!NtUserRegisterTouchPadCapable(TRUE));
            }
            else [[unlikely]]
            {
                LOG_HR_MSG(E_FAIL, "Touchpad awareness cannot be enabled due to neither RegisterTouchpadCapableThread function nor NtUserRegisterTouchPadCapable function being found on the current OS environment.");
            }
        }

        winrt::check_pointer(m_hwnd = CreateWindowExW(
            dwmFrameEnabled ?
                static_cast<uint32_t>(m_extendedStyles) :
                (static_cast<uint32_t>(m_extendedStyles) &~ (WS_EX_NOREDIRECTIONBITMAP | WS_EX_DLGMODALFRAME)),
            className,
            m_title.c_str(),
            static_cast<uint32_t>(m_styles) &~ WS_VISIBLE,
            options.Left(),
            options.Top(),
            options.Width(),
            options.Height(),
            NULL,
            NULL,
            GetModuleHandleW(nullptr),
            nullptr));

        winrt::check_hresult(PrivateCreateCoreWindow(
            IMMERSIVE_HOSTED,
            L"",
            0, 0, 0, 0,
            0,
            m_hwnd,
            winrt::guid_of<ICoreWindow>(),
            winrt::put_abi(m_coreWindow)));

        winrt::com_ptr<ICoreWindowInterop> interop = m_coreWindow.as<ICoreWindowInterop>();
        winrt::check_hresult(interop->get_WindowHandle(&m_coreWindowHwnd));

        m_dispatcher = m_coreWindow.Dispatcher();

        if (Features::IsDispatcherQueueAvailable)
            m_dispatcherQueue = m_coreWindow.DispatcherQueue();

        SetPropW(m_hwnd, XHK_WINDOW_OBJECT_PROP, this);

        if (XamlConfig::s_enableTouchpadAwareness && RegisterTouchpadCapableWindowMethod) [[unlikely]]
        {
            RegisterTouchpadCapableWindowMethod(m_hwnd, TRUE);
            RegisterTouchpadCapableWindowMethod(m_coreWindowHwnd, TRUE);
        }

        if (SetWindowCompositionAttribute && Helpers::OSBuild >= 17763u) [[likely]]
        {
            ACCENT_POLICY policy { ACCENT_ENABLE_HOSTBACKDROP };

            WINDOWCOMPOSITIONATTRIBDATA data = { };
            data.Attrib = WINDOWCOMPOSITIONATTRIB::WCA_ACCENT_POLICY;
            data.pvData = &policy;
            data.cbData = sizeof(policy);

            LOG_LAST_ERROR_IF(!SetWindowCompositionAttribute(m_hwnd, &data));
            SetWindowCompositionAttribute(m_coreWindowHwnd, &data);
        }
        else
        {
            LOG_HR_MSG(E_FAIL, "Host Backrop cannot be enabled due to SetWindowCompositionAttribute function not being found on (or not compatible with) the current OS environment.");
        }

        ::IUnknown* pView = nullptr;
        if (auto cap2 = winrt::try_get_activation_factory<CoreApplication, ICoreApplicationPrivate2>()) [[likely]]
        {
            LOG_IF_FAILED(cap2->CreateNonImmersiveView((void**)&pView));
        }

        m_view = winrt::make<LegacyNonImmersiveView>(m_coreWindow, isMain, pView)
            .as<winrt::Windows::ApplicationModel::Core::CoreApplicationView>();

        m_frameworkView = { };
        m_frameworkView.Initialize(m_view);
        m_frameworkView.SetWindow(m_coreWindow);
        m_systemWindow = Window::Current();

        if (auto wPriv = m_systemWindow.try_as<IWindowPrivate>()) [[likely]]
        {
            LOG_IF_FAILED(wPriv->put_TransparentBackground(TRUE));

            if (XamlConfig::s_enableSmoothResize && Features::IsSetSynchronizationInfoAvailable) [[likely]]
            {
                if (!LOG_LAST_ERROR_IF(!Helpers::EnableResizeSynchronization(m_hwnd, true))) [[likely]]
                {
                    Helpers::EnableResizeSynchronization(m_coreWindowHwnd, true);
                    m_windowPrivate.attach(wPriv.detach());
                    m_isSyncObjEnabled = true;

                    if (HANDLE syncHandle = Helpers::GetResizeSynchronizationObject(m_hwnd)) [[unlikely]]
                    {
                        LOG_IF_FAILED(m_windowPrivate->SetSynchronizationInfo((uint64_t)syncHandle, (uint64_t)m_hwnd));
                        CloseHandle(syncHandle);
                    }
                }
            }
        }

        RECT clientRect { };
        GetClientRect(m_hwnd, &clientRect);

        SetParent(m_coreWindowHwnd, m_hwnd);
        SetWindowLongW(m_coreWindowHwnd, GWL_STYLE, WS_CHILD | WS_VISIBLE);
        SetWindowPos(m_coreWindowHwnd, NULL, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);

        s_currentWindow = this;
    }

    winrt::XamlHostingKit::XamlWindow XamlWindow::Current()
    {
        if (auto window = s_currentWindow) [[likely]]
        {
            return *window;
        }

        return nullptr;
    }

    hstring XamlWindow::Title()
    {
        return m_title;
    }

    void XamlWindow::Title(hstring const& value)
    {
        m_title = value;
        SetWindowTextW(m_hwnd, value.c_str());
    }

    winrt::Windows::Foundation::Rect XamlWindow::Bounds()
    {
        RECT rect { };
        if (!IsIconic(m_hwnd)) [[likely]]
        {
            GetWindowRect(m_hwnd, &rect);
        }
        else
        {
            WINDOWPLACEMENT placement;
            placement.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement(m_hwnd, &placement);
            rect = placement.rcNormalPosition;
        }

        auto dpi = Helpers::GetDpiScaleForWindow(m_hwnd);

        return
        {
            static_cast<float>(rect.left) / dpi,
            static_cast<float>(rect.top) / dpi,
            static_cast<float>(rect.right - rect.left) / dpi,
            static_cast<float>(rect.bottom - rect.top) / dpi
        };
    }

    WindowStyles XamlWindow::Styles()
    {
        return m_styles;
    }

    void XamlWindow::Styles(WindowStyles value)
    {
        SetWindowLongPtrW(m_hwnd, GWL_STYLE, static_cast<uint32_t>(value));
        m_styles = static_cast<WindowStyles>(GetWindowLongPtrW(m_hwnd, GWL_STYLE));
    }

    WindowExtendedStyles XamlWindow::ExtendedStyles()
    {
        return m_extendedStyles;
    }

    void XamlWindow::ExtendedStyles(WindowExtendedStyles value)
    {
        SetWindowLongPtrW(m_hwnd, GWL_EXSTYLE, static_cast<uint32_t>(value));
        m_extendedStyles = static_cast<WindowExtendedStyles>(GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE));
    }

    bool XamlWindow::IsVisible()
    {
        return IsWindowVisible(m_hwnd) != FALSE;
    }

    winrt::Windows::UI::WindowId XamlWindow::WindowHandle()
    {
        return { (uint64_t)m_hwnd };
    }

    winrt::Windows::UI::Xaml::Window XamlWindow::SystemWindow()
    {
        return m_systemWindow;
    }

    winrt::Windows::ApplicationModel::Core::CoreApplicationView XamlWindow::CoreApplicationView()
    {
        return m_view;
    }

    winrt::Windows::UI::Core::CoreDispatcher XamlWindow::Dispatcher()
    {
        return m_dispatcher;
    }

    winrt::Windows::System::DispatcherQueue XamlWindow::DispatcherQueue()
    {
        return m_dispatcherQueue;
    }

    winrt::Windows::UI::Core::CoreWindow XamlWindow::CoreWindow()
    {
        return m_coreWindow;
    }

    winrt::Windows::UI::Composition::Compositor XamlWindow::Compositor()
    {
        if (auto window3 = m_systemWindow.try_as<IWindow3>()) [[likely]]
            return window3.Compositor();

        return nullptr;
    }

    winrt::Windows::UI::UIContext XamlWindow::UIContext()
    {
        if (auto window4 = m_systemWindow.try_as<IWindow4>()) [[likely]]
            return window4.UIContext();

        return nullptr;
    }

    winrt::Windows::UI::Xaml::UIElement XamlWindow::Content()
    {
        return m_systemWindow.Content();
    }

    void XamlWindow::Content(winrt::Windows::UI::Xaml::UIElement const& value)
    {
        m_systemWindow.Content(value);
    }

    void XamlWindow::Show()
    {
        ShowWindow(m_hwnd, SW_SHOW);
        SetForegroundWindow(m_hwnd);
        SetActiveWindow(m_coreWindowHwnd);
        SetActiveWindow(m_hwnd);
    }

    void XamlWindow::Hide()
    {
        ShowWindow(m_hwnd, SW_HIDE);
    }

    void XamlWindow::Close()
    {
        CloseWindow(m_hwnd);
    }

    void XamlWindow::Move(winrt::Windows::Foundation::Point topleft)
    {
        Move(topleft.X, topleft.Y);
    }

    void XamlWindow::Move(float left, float top)
    {
        auto dpi = Helpers::GetDpiScaleForWindow(m_hwnd);

        if (!IsZoomed(m_hwnd) && !IsIconic(m_hwnd)) [[likely]]
        {
            SetWindowPos(m_hwnd, NULL,
                         static_cast<int>(left * dpi),
                         static_cast<int>(top * dpi),
                         0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        else
        {
            WINDOWPLACEMENT wp;
            wp.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement(m_hwnd, &wp);

            LONG width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
            LONG height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;

            wp.rcNormalPosition.left = static_cast<int>(left * dpi);
            wp.rcNormalPosition.top = static_cast<int>(top * dpi);
            wp.rcNormalPosition.right = static_cast<int>(left * dpi + width);
            wp.rcNormalPosition.bottom = static_cast<int>(top * dpi + height);
            SetWindowPlacement(m_hwnd, &wp);
        }
    }

    void XamlWindow::Resize(winrt::Windows::Foundation::Size size)
    {
        Resize(size.Width, size.Height);
    }

    void XamlWindow::Resize(float width, float height)
    {
        auto dpi = Helpers::GetDpiScaleForWindow(m_hwnd);

        if (!IsZoomed(m_hwnd) && !IsIconic(m_hwnd)) [[likely]]
        {
            SetWindowPos(m_hwnd, NULL, 0, 0,
                         static_cast<int>(width * dpi),
                         static_cast<int>(height * dpi),
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        else
        {
            WINDOWPLACEMENT wp;
            wp.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement(m_hwnd, &wp);

            wp.rcNormalPosition.right = static_cast<int>(wp.rcNormalPosition.left + width * dpi);
            wp.rcNormalPosition.bottom = static_cast<int>(wp.rcNormalPosition.top + height * dpi);
            SetWindowPlacement(m_hwnd, &wp);
        }
    }

    winrt::event_token XamlWindow::VisibilityChanged(winrt::Windows::Foundation::TypedEventHandler<winrt::XamlHostingKit::XamlWindow, bool> const& handler)
    {
        return m_visibilityChanged.add(handler);
    }

    void XamlWindow::VisibilityChanged(winrt::event_token const& token) noexcept
    {
        m_visibilityChanged.remove(token);
    }

    void XamlWindow::RunMessageLoop()
    {
        m_frameworkView.Run();
        
        if (!XamlConfig::s_disableEarlyXamlShutdown) [[likely]]
        {
            m_frameworkView.Uninitialize();
        }
    }

    HRESULT XamlWindow::get_WindowHandle(HWND* hwnd)
    {
        if (hwnd == nullptr) [[unlikely]]
        {
            return E_POINTER;
        }

        *hwnd = m_hwnd;
        return S_OK;
    }

    LPCWSTR const XamlWindow::RegisterWindowClass(WNDPROC wndProc)
    {
        const auto constexpr className = L"COMplicated.XamlHostingKit.Window";

        WNDCLASSEXW wcex = { };
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = XamlConfig::s_disableRedirectionLayer ? NULL : CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = wndProc;
        wcex.hInstance = GetModuleHandleW(nullptr);
        wcex.lpszClassName = className;

        ExtractIconExW(Helpers::GetExecutablePath(), 0, s_iconLarge.put(), s_iconSmall.put(), 1);
        wcex.hIcon = s_iconLarge.get();
        wcex.hIconSm = s_iconSmall.get();

        if (!RegisterClassExW(&wcex)) [[unlikely]]
        {
            throw winrt::hresult_error(HRESULT_FROM_WIN32(GetLastError()), L"Failed to register window class");
        }

        return className;
    }

    LRESULT XamlWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        auto _this = reinterpret_cast<XamlWindow*>(GetPropW(hwnd, XHK_WINDOW_OBJECT_PROP));

        if (msg == WM_CREATE)
        {
            Helpers::EnableDarkModeSupport(hwnd);
            Helpers::EnsureTitleBarTheme(hwnd);
        }
        else if (msg == WM_SETTINGCHANGE)
        {
            if ((BOOL)lParam && wcscmp((wchar_t*)lParam, L"ImmersiveColorSet") == 0)
                Helpers::EnsureTitleBarTheme(hwnd);

            if (_this) [[likely]]
            {
                SendMessageW(_this->m_coreWindowHwnd, msg, wParam, lParam);
            }
        }
        else if (msg == WM_DESTROY)
        {
            RemovePropW(hwnd, XHK_WINDOW_OBJECT_PROP);

            s_currentWindow = nullptr;
            PostQuitMessage(0);

            if (_this) [[likely]]
            {
                /*try
                {
                    _this->m_dispatcher.StopProcessEvents();
                }
                catch (...) { }*/

                XamlApplication::RemoveWindow(*_this);
            }
        }
        else if (_this)
        {
            if (msg == WM_SIZE)
            {
                // TODO: Should this be after SetSynchronizationInfo instead?
                // this is currently here before it because XAML sets the CoreWindow synchronization object
                // on every WM_SIZE message, so it seems like we should send that message before we set
                // the synchronization object so that XAML doesn't override it (with CoreWindow's one),
                // but also the code around (NtUser)LayoutCompleted in WUX.dll seems to suggest that we are supposed to resize
                // after setting the synchronization object, this is so confusing...
                SetWindowPos(_this->m_coreWindowHwnd, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);
                SendMessageW(_this->m_coreWindowHwnd, msg, wParam, lParam);

                if (_this->m_isSyncObjEnabled) [[likely]]
                {
                    if (HANDLE syncHandle = Helpers::GetResizeSynchronizationObject(hwnd)) [[likely]]
                    {
                        _this->m_windowPrivate->SetSynchronizationInfo((uint64_t)syncHandle, (uint64_t)hwnd);
                        CloseHandle(syncHandle);
                    }
                }
            }
            else if (msg == WM_SHOWWINDOW && _this->m_visibilityChanged)
            {
                _this->m_dispatcher.RunAsync(CoreDispatcherPriority::Normal, [=]()
                {
                    _this->m_visibilityChanged(*_this, wParam != FALSE);
                });
            }
            else if (msg == WM_SETFOCUS)
            {
                SetFocus(_this->m_coreWindowHwnd);
            }
            else if (msg == WM_THEMECHANGED || msg == WM_ACTIVATE)
            {
                SendMessageW(_this->m_coreWindowHwnd, msg, wParam, lParam);
            }
            else if (msg == WM_DWMNCRENDERINGCHANGED)
            {
                SetWindowLongW(hwnd, GWL_EXSTYLE, (BOOL)wParam ?
                    static_cast<uint32_t>(_this->ExtendedStyles()) :
                    static_cast<uint32_t>(_this->ExtendedStyles()) &~ (WS_EX_NOREDIRECTIONBITMAP | WS_EX_DLGMODALFRAME));
            }
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}
