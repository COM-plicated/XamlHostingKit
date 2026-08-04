#pragma once
#include "XamlConfig.g.h"

namespace winrt::XamlHostingKit::implementation
{
    struct XamlConfig
    {
        static bool s_isInitialized;
        static bool s_enableWebView;
        static bool s_disableRedirectionLayer;
        static bool s_enableTouchpadAwareness;

        XamlConfig() = default;

        static bool EnableWebView();
        static void EnableWebView(bool value);

        static bool DisableRedirectionLayer();
        static void DisableRedirectionLayer(bool value);

        static bool EnableTouchpadAwareness();
        static void EnableTouchpadAwareness(bool value);
    };
}

namespace winrt::XamlHostingKit::factory_implementation
{
    struct XamlConfig : XamlConfigT<XamlConfig, implementation::XamlConfig>
    {

    };
}
