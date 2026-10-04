#pragma once

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

    public:
        WindowCreationOptions() = default;

        hstring Title();
        void Title(hstring const& value);

        Windows::Foundation::IReference<float> Left() const;
        void Left(Windows::Foundation::IReference<float> value);

        Windows::Foundation::IReference<float> Top() const;
        void Top(Windows::Foundation::IReference<float> value);

        Windows::Foundation::IReference<float> Width() const;
        void Width(Windows::Foundation::IReference<float> value);

        Windows::Foundation::IReference<float> Height() const;
        void Height(Windows::Foundation::IReference<float> value);

        WindowStyles Styles() const;
        void Styles(WindowStyles value);

        WindowExtendedStyles ExtendedStyles() const;
        void ExtendedStyles(WindowExtendedStyles value);
    };
}

namespace winrt::XamlHostingKit::factory_implementation
{
    struct WindowCreationOptions : WindowCreationOptionsT<WindowCreationOptions, implementation::WindowCreationOptions>
    {

    };
}
