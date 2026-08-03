#include "pch.h"
#include "XamlWindow.h"
#include "XamlWindow.g.cpp"

#include "XamlConfig.h"
#include "Helpers.h"

namespace winrt::XamlHostingKit::implementation
{
    const LPCWSTR XamlWindow::s_windowClassName = RegisterWindowClass(WndProc);
    thread_local XamlWindow* XamlWindow::s_currentWindow = nullptr;

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
        GetWindowRect(m_hwnd, &rect);
        uint32_t dpi = Helpers::GetDpiForWindow(m_hwnd);

        return
        {
            static_cast<float>(rect.left) / dpi,
            static_cast<float>(rect.top) / dpi,
            static_cast<float>(rect.right - rect.left) / dpi,
            static_cast<float>(rect.bottom - rect.top) / dpi
        };
    }

    void XamlWindow::Bounds(winrt::Windows::Foundation::Rect const& value)
    {
        uint32_t dpi = Helpers::GetDpiForWindow(m_hwnd);
        SetWindowPos(m_hwnd, NULL, value.X * dpi, value.Y * dpi, value.Width * dpi, value.Height * dpi, SWP_NOZORDER | SWP_NOACTIVATE);
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

        try
        {
            m_dispatcher.StopProcessEvents();
        }
        catch (...)
        {
            // nothing to do here, WM_DESTROY -> PostQuitMessage will take care of the message loop
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
            if ((BOOL)lParam && std::wstring_view((wchar_t*)lParam) == L"ImmersiveColorSet")
                Helpers::EnsureTitleBarTheme(hwnd);

            if (_this)
            {
                SendMessageW(_this->m_coreWindowHwnd, msg, wParam, lParam);
            }
        }
        else if (msg == WM_DESTROY)
        {
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
