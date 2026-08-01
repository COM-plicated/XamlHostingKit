#pragma once

#include "WindowCreationOptions.g.h"
#include "Helpers.h"

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
        std::uint32_t m_styles         { WS_OVERLAPPEDWINDOW };
        std::uint32_t m_extendedStyles { WS_EX_NOREDIRECTIONBITMAP | WS_EX_DLGMODALFRAME };

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

        std::uint32_t Styles() const;
        void Styles(std::uint32_t value);

        std::uint32_t ExtendedStyles() const;
        void ExtendedStyles(std::uint32_t value);
    };
}

namespace winrt::XamlHostingKit::factory_implementation
{
    struct WindowCreationOptions : WindowCreationOptionsT<WindowCreationOptions, implementation::WindowCreationOptions>
    {

    };
}
