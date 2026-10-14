#pragma once

#include <winrt/Windows.UI.WindowManagement.h>

#include "WindowCreationOptions.g.h"
#include "Helpers.h"

#include "XamlConfig.h"

namespace winrt::XamlHostingKit::implementation
{
    struct WindowCreationOptions : WindowCreationOptionsT<WindowCreationOptions>
    {
    private:
        hstring                                m_title          { Helpers::GetExecutableName() };
        Windows::Foundation::IReference<float> m_left           { nullptr };
        Windows::Foundation::IReference<float> m_top            { nullptr };
        Windows::Foundation::IReference<float> m_width          { nullptr };
        Windows::Foundation::IReference<float> m_height         { nullptr };
        WindowStyles                           m_styles         { WindowStyles::OverlappedWindow };
        WindowExtendedStyles                   m_extendedStyles
        { 
            XamlConfig::s_disableRedirectionLayer ?
                WindowExtendedStyles::NoRedirectionBitmap | WindowExtendedStyles::DialogModalFrame :
                WindowExtendedStyles::DialogModalFrame
        };
        Windows::Foundation::IReference<winrt::Windows::Graphics::DisplayId> m_monitorHandle { nullptr };
        Windows::UI::WindowManagement::DisplayRegion m_displayRegion { nullptr };

    public:
        WindowCreationOptions() = default;

        hstring Title();
        void Title(hstring const& value);

        Windows::Foundation::IReference<float> Left() const;
        void Left(Windows::Foundation::IReference<float> const& value);

        Windows::Foundation::IReference<float> Top() const;
        void Top(Windows::Foundation::IReference<float> const& value);

        Windows::Foundation::IReference<float> Width() const;
        void Width(Windows::Foundation::IReference<float> const& value);

        Windows::Foundation::IReference<float> Height() const;
        void Height(Windows::Foundation::IReference<float> const& value);

        WindowStyles Styles() const;
        void Styles(WindowStyles const& value);

        WindowExtendedStyles ExtendedStyles() const;
        void ExtendedStyles(WindowExtendedStyles const& value);

        Windows::Foundation::IReference<winrt::Windows::Graphics::DisplayId> MonitorHandle() const;
        void MonitorHandle(Windows::Foundation::IReference<winrt::Windows::Graphics::DisplayId> const& value);

        Windows::UI::WindowManagement::DisplayRegion DisplayRegion() const;
        void DisplayRegion(Windows::UI::WindowManagement::DisplayRegion const& value);
    };
}

namespace winrt::XamlHostingKit::factory_implementation
{
    struct WindowCreationOptions : WindowCreationOptionsT<WindowCreationOptions, implementation::WindowCreationOptions>
    {

    };
}
