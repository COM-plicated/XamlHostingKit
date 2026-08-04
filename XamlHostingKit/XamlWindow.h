#pragma once
#include "XamlWindow.g.h"
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.h>
#include "IWindowNative.h"

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

        bool m_isMain { false };
        HWND m_hwnd { nullptr };
        HWND m_coreWindowHwnd { nullptr };
        hstring m_title;
        uint32_t m_styles { 0 };
        uint32_t m_extendedStyles { 0 };
        Window m_systemWindow { nullptr };
        CoreApplicationView m_view { nullptr };
        CoreDispatcher m_dispatcher { nullptr };
        DispatcherQueue m_dispatcherQueue { nullptr };
        CoreWindow m_coreWindow { nullptr };
        FrameworkView m_frameworkView { nullptr };
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

        std::uint32_t Styles();
        void Styles(std::uint32_t value);

        std::uint32_t ExtendedStyles();
        void ExtendedStyles(std::uint32_t value);

        bool IsVisible();
        winrt::Windows::UI::WindowId WindowHandle();
        winrt::Windows::UI::Xaml::Window SystemWindow();
        winrt::Windows::ApplicationModel::Core::CoreApplicationView CoreApplicationView();

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
