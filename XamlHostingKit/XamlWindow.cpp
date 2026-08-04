#include "pch.h"
#include "XamlWindow.h"
#include "XamlWindow.g.cpp"

#include <dwmapi.h>
#include "XamlConfig.h"
#include "Helpers.h"
#include "LegacyNonImmersiveView.h"
#include <CoreWindow.h>
#include "Features.h"
#include "XamlApplication.h"

namespace winrt::XamlHostingKit::implementation
{
    const LPCWSTR XamlWindow::s_windowClassName = RegisterWindowClass(WndProc);
    thread_local XamlWindow* XamlWindow::s_currentWindow = nullptr;

    XamlWindow::XamlWindow(winrt::XamlHostingKit::WindowCreationOptions const& options, bool isMain)
        : m_isMain(isMain)
    {
        m_title = options.Title();
        m_styles = options.Styles();
        m_extendedStyles = options.ExtendedStyles();

        BOOL dwmFrameEnabled = TRUE;
        if (FAILED(DwmIsCompositionEnabled(&dwmFrameEnabled)))
        {
            dwmFrameEnabled = TRUE;
        }

        winrt::check_pointer(m_hwnd = CreateWindowExW(
            dwmFrameEnabled ?
                m_extendedStyles :
                (m_extendedStyles |~ (WS_EX_NOREDIRECTIONBITMAP | WS_EX_DLGMODALFRAME)),
            s_windowClassName,
            m_title.c_str(),
            m_styles |~ WS_VISIBLE,
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

        if (Features::IsDispatcherQueueSupported)
            m_dispatcherQueue = m_coreWindow.DispatcherQueue();

        SetPropW(m_hwnd, XHK_WINDOW_OBJECT_PROP, this);

        if (SetWindowCompositionAttribute)
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
            LOG_HR_MSG(E_FAIL, "Host Backrop cannot be enabled due to SetWindowCompositionAttribute function not being found on the current OS environment.");
        }

        ::IUnknown* pView = nullptr;
        if (auto cap2 = winrt::try_get_activation_factory<CoreApplication, ICoreApplicationPrivate2>())
        {
            LOG_IF_FAILED(cap2->CreateNonImmersiveView((void**)&pView));
        }

        m_view = winrt::make<LegacyNonImmersiveView>(m_coreWindow, isMain, pView)
            .as<winrt::Windows::ApplicationModel::Core::CoreApplicationView>();

        m_frameworkView = { };
        m_frameworkView.Initialize(m_view);
        m_frameworkView.SetWindow(m_coreWindow);

        RECT clientRect { };
        GetClientRect(m_hwnd, &clientRect);

        SetParent(m_coreWindowHwnd, m_hwnd);
        SetWindowLongW(m_coreWindowHwnd, GWL_STYLE, WS_CHILD | WS_VISIBLE);
        SetWindowPos(m_coreWindowHwnd, NULL, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);

        s_currentWindow = this;
    }

    winrt::XamlHostingKit::XamlWindow XamlWindow::Current()
    {
        if (auto window = s_currentWindow)
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
        if (!IsIconic(m_hwnd))
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

    std::uint32_t XamlWindow::Styles()
    {
        return m_styles;
    }

    void XamlWindow::Styles(std::uint32_t value)
    {
        SetWindowLongPtrW(m_hwnd, GWL_STYLE, value);
        m_styles = static_cast<uint32_t>(GetWindowLongPtrW(m_hwnd, GWL_STYLE));
    }

    std::uint32_t XamlWindow::ExtendedStyles()
    {
        return m_extendedStyles;
    }

    void XamlWindow::ExtendedStyles(std::uint32_t value)
    {
        SetWindowLongPtrW(m_hwnd, GWL_EXSTYLE, value);
        m_extendedStyles = static_cast<uint32_t>(GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE));
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
        return m_systemWindow.Compositor();
    }

    winrt::Windows::UI::UIContext XamlWindow::UIContext()
    {
        return m_systemWindow.UIContext();
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

        if (!IsZoomed(m_hwnd) && !IsIconic(m_hwnd))
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

        if (!IsZoomed(m_hwnd) && !IsIconic(m_hwnd))
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
    }

    HRESULT XamlWindow::get_WindowHandle(HWND* hwnd)
    {
        if (hwnd == nullptr)
            return E_POINTER;

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

        if (!RegisterClassExW(&wcex))
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

            if (_this)
            {
                SendMessageW(_this->m_coreWindowHwnd, msg, wParam, lParam);
            }
        }
        else if (msg == WM_DESTROY)
        {
            RemovePropW(hwnd, XHK_WINDOW_OBJECT_PROP);

            s_currentWindow = nullptr;

            if (_this)
            {
                try
                {
                    _this->m_frameworkView.Uninitialize();
                }
                catch (...) { }

                try
                {
                    _this->m_dispatcher.StopProcessEvents();
                }
                catch (...) { }

                XamlApplication::RemoveWindow(*_this);
            }

            PostQuitMessage(0);
        }
        else if (_this)
        {
            if (msg == WM_SIZE)
            {
                SetWindowPos(_this->m_coreWindowHwnd, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);
                SendMessageW(_this->m_coreWindowHwnd, msg, wParam, lParam);
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
            else if (msg == WM_THEMECHANGED)
            {
                SendMessageW(_this->m_coreWindowHwnd, msg, wParam, lParam);
            }
            else if (msg == WM_DWMNCRENDERINGCHANGED)
            {
                SetWindowLongW(hwnd, GWL_EXSTYLE, (BOOL)wParam ?
                    _this->ExtendedStyles() :
                    _this->ExtendedStyles() |~ (WS_EX_NOREDIRECTIONBITMAP | WS_EX_DLGMODALFRAME));
            }
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}
