#pragma once
#include "XamlConfig.g.h"

namespace winrt::XamlHostingKit::implementation
{
    struct XamlConfig
    {
    private:
        static bool s_enableWebView;
        static bool s_disableRedirectionLayer;

    public:
        static bool IsInitialized;

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
