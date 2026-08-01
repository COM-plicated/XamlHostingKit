#pragma once
#include "WindowCreationOptions.g.h"

namespace winrt::XamlHostingKit::implementation
{
    struct WindowCreationOptions : WindowCreationOptionsT<WindowCreationOptions>
    {
        WindowCreationOptions() = default;

        hstring Title();
        void Title(hstring const& value);

        std::int32_t Left();
        void Left(std::int32_t value);

        std::int32_t Top();
        void Top(std::int32_t value);

        std::int32_t Width();
        void Width(std::int32_t value);

        std::int32_t Height();
        void Height(std::int32_t value);

        std::uint32_t Styles();
        void Styles(std::uint32_t value);

        std::uint32_t ExtendedStyles();
        void ExtendedStyles(std::uint32_t value);
    };
}

namespace winrt::XamlHostingKit::factory_implementation
{
    struct WindowCreationOptions : WindowCreationOptionsT<WindowCreationOptions, implementation::WindowCreationOptions>
    {

    };
}
