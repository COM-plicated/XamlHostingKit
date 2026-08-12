#pragma once

#include "WindowCreationOptions.g.h"
#include "Helpers.h"

#include "XamlConfig.h"

namespace winrt::XamlHostingKit::implementation
{
    struct WindowCreationOptions : WindowCreationOptionsT<WindowCreationOptions>
    {
    private:
        hstring       m_title          { Helpers::GetExecutableName() };
        std::int32_t  m_left           { CW_USEDEFAULT };
        std::int32_t  m_top            { CW_USEDEFAULT };
        std::int32_t  m_width          { CW_USEDEFAULT };
        std::int32_t  m_height         { CW_USEDEFAULT };
        WindowStyles  m_styles         { WindowStyles::OverlappedWindow };
        WindowExtendedStyles m_extendedStyles
        { 
            XamlConfig::s_disableRedirectionLayer ?
                WindowExtendedStyles::NoRedirectionBitmap | WindowExtendedStyles::DialogModalFrame :
                WindowExtendedStyles::DialogModalFrame
        };

    public:
        WindowCreationOptions() = default;

        hstring Title();
        void Title(hstring const& value);

        std::int32_t Left() const;
        void Left(std::int32_t value);

        std::int32_t Top() const;
        void Top(std::int32_t value);

        std::int32_t Width() const;
        void Width(std::int32_t value);

        std::int32_t Height() const;
        void Height(std::int32_t value);

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
