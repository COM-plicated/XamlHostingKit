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
        static bool s_disableEarlyXamlShutdown;
        static bool s_enableMsAppxWebProtocolSupport;
        static bool s_enableArbitraryPathsInMsAppxWeb;
        static bool s_enableSmoothResize;

        XamlConfig() = default;

        static bool EnableWebView();
        static void EnableWebView(bool value);

        static bool DisableRedirectionLayer();
        static void DisableRedirectionLayer(bool value);

        static bool EnableTouchpadAwareness();
        static void EnableTouchpadAwareness(bool value);

        static bool DisableEarlyXamlShutdown();
        static void DisableEarlyXamlShutdown(bool value);

        static bool EnableMsAppxWebProtocolSupport();
        static void EnableMsAppxWebProtocolSupport(bool value);

        static bool EnableArbitraryPathsInMsAppxWeb();
        static void EnableArbitraryPathsInMsAppxWeb(bool value);

        static bool EnableSmoothResize();
        static void EnableSmoothResize(bool value);
    };
}

namespace winrt::XamlHostingKit::factory_implementation
{
    struct XamlConfig : XamlConfigT<XamlConfig, implementation::XamlConfig>
    {

    };
}
