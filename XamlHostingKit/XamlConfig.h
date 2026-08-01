#pragma once
#include "XamlConfig.g.h"

namespace winrt::XamlHostingKit::implementation
{
    struct XamlConfig
    {
        XamlConfig() = default;

        static bool EnableWebView();
        static void EnableWebView(bool value);

        static bool DisableRedirectionLayer();
        static void DisableRedirectionLayer(bool value);
    };
}

namespace winrt::XamlHostingKit::factory_implementation
{
    struct XamlConfig : XamlConfigT<XamlConfig, implementation::XamlConfig>
    {

    };
}
