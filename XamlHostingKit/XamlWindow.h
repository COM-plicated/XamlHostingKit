#pragma once
#include "XamlWindow.g.h"
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.h>
#include "IWindowNative.h"
#include "Helpers.h"

namespace winrt::XamlHostingKit::implementation
{
    using namespace winrt::Windows::System;
    using namespace winrt::Windows::UI::Xaml;
    using namespace winrt::Windows::UI::Core;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::ApplicationModel::Core;

    struct XamlWindow : XamlWindowT<XamlWindow, IWindowNative>
    {
    private:
        static thread_local XamlWindow* s_currentWindow;
        static const constexpr auto XHK_WINDOW_OBJECT_PROP = L"COMplicated.XamlHostingKit.WindowObject";

        inline static wil::unique_hicon s_iconSmall { nullptr };
        inline static wil::unique_hicon s_iconLarge { nullptr };

        bool m_isMain { false };
        bool m_isSyncObjEnabled { false };
        bool m_isDestroyed { false };
        HWND m_hwnd { nullptr };
        HWND m_coreWindowHwnd { nullptr };
        hstring m_title;
        WindowStyles m_styles { 0 };
        WindowExtendedStyles m_extendedStyles { 0 };
        Window m_systemWindow { nullptr };
        CoreApplicationView m_view { nullptr };
        CoreDispatcher m_dispatcher { nullptr };
        DispatcherQueue m_dispatcherQueue { nullptr };
        CoreWindow m_coreWindow { nullptr };
        FrameworkView m_frameworkView { nullptr };
        winrt::XamlHostingKit::XamlTitleBar m_titleBar { nullptr };
        winrt::com_ptr<IWindowPrivate> m_windowPrivate { nullptr };
        winrt::com_ptr<IFrameworkApplicationPrivate> m_applicationPrivate { nullptr };
        winrt::com_ptr<IFrameworkApplicationPrivateOld> m_applicationPrivateOld { nullptr };
        winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::XamlHostingKit::XamlWindow, bool>> m_visibilityChanged;

        static LPCWSTR const RegisterWindowClass(WNDPROC wndProc);
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    public:


        XamlWindow() = default;
        XamlWindow(winrt::XamlHostingKit::WindowCreationOptions const& options, bool isMain = false);

        static winrt::XamlHostingKit::XamlWindow Current();

        hstring Title();
        void Title(hstring const& value);

        winrt::Windows::Foundation::Rect Bounds();

        WindowStyles Styles();
        void Styles(WindowStyles value);

        WindowExtendedStyles ExtendedStyles();
        void ExtendedStyles(WindowExtendedStyles value);

        bool IsVisible();
        winrt::Windows::UI::WindowId WindowHandle();
        winrt::Windows::UI::Xaml::Window SystemWindow();
        winrt::Windows::ApplicationModel::Core::CoreApplicationView CoreApplicationView();

        winrt::XamlHostingKit::XamlTitleBar TitleBar();
        winrt::Windows::UI::Core::CoreDispatcher Dispatcher();
        winrt::Windows::System::DispatcherQueue DispatcherQueue();
        winrt::Windows::UI::Core::CoreWindow CoreWindow();
        winrt::Windows::UI::Composition::Compositor Compositor();
        winrt::Windows::UI::UIContext UIContext();

        winrt::Windows::UI::Xaml::UIElement Content();
        void Content(winrt::Windows::UI::Xaml::UIElement const& value);

        void Show();
        void Hide();
        void Close();

        void Move(winrt::Windows::Foundation::Point topleft);
        void Move(float left, float top);
        void Resize(winrt::Windows::Foundation::Size size);
        void Resize(float width, float height);

        winrt::event_token VisibilityChanged(winrt::Windows::Foundation::TypedEventHandler<winrt::XamlHostingKit::XamlWindow, bool> const& handler);
        void VisibilityChanged(winrt::event_token const& token) noexcept;

        HRESULT STDMETHODCALLTYPE get_WindowHandle(HWND* hwnd);

        void RunMessageLoop();
    };
}

namespace winrt::XamlHostingKit::factory_implementation
{
    struct XamlWindow : XamlWindowT<XamlWindow, implementation::XamlWindow>
    {

    };
}
