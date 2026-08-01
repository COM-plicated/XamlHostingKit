#pragma once
#include "XamlWindow.g.h"

namespace winrt::XamlHostingKit::implementation
{
    struct XamlWindow : XamlWindowT<XamlWindow>
    {
        XamlWindow() = default;

        static winrt::XamlHostingKit::XamlWindow Current();

        hstring Title();
        void Title(hstring const& value);

        winrt::Windows::Foundation::Rect Bounds();
        void Bounds(winrt::Windows::Foundation::Rect const& value);

        std::uint32_t Styles();
        void Styles(std::uint32_t value);

        std::uint32_t ExtendedStyles();
        void ExtendedStyles(std::uint32_t value);

        bool IsVisible();
        winrt::Windows::UI::WindowId WindowHandle();
        winrt::Windows::UI::Xaml::Window SystemWindow();
        winrt::Windows::ApplicationModel::Core::CoreApplicationView View();

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

        winrt::event_token VisibilityChanged(winrt::Windows::Foundation::TypedEventHandler<winrt::XamlHostingKit::XamlWindow, bool> const& handler);
        void VisibilityChanged(winrt::event_token const& token) noexcept;
    };
}

namespace winrt::XamlHostingKit::factory_implementation
{
    struct XamlWindow : XamlWindowT<XamlWindow, implementation::XamlWindow>
    {

    };
}
