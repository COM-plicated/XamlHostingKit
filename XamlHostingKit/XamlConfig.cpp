#include "pch.h"
#include "XamlConfig.h"
#include "XamlConfig.g.cpp"
#include "Helpers.h"

namespace winrt::XamlHostingKit::implementation
{
    bool XamlConfig::s_isInitialized = false;
    bool XamlConfig::s_enableWebView = true;
    bool XamlConfig::s_disableRedirectionLayer = true;
    bool XamlConfig::s_enableTouchpadAwareness = true;
    bool XamlConfig::s_disableEarlyXamlShutdown = false;
    bool XamlConfig::s_enableMsAppxWebProtocolSupport = false;
    bool XamlConfig::s_enableArbitraryPathsInMsAppxWeb = false;

    bool XamlConfig::EnableWebView()
    {
        return s_enableWebView;
    }

    void XamlConfig::EnableWebView(bool value)
    {
        if (s_isInitialized) [[unlikely]]
        {
            throw hresult_illegal_method_call
            (L"Changing EnableWebView after calling XamlApplication.Start() is not supported.");
        }

        s_enableWebView = value;
    }

    bool XamlConfig::DisableRedirectionLayer()
    {
        return s_disableRedirectionLayer;
    }

    void XamlConfig::DisableRedirectionLayer(bool value)
    {
        if (s_isInitialized) [[unlikely]]
        {
            throw hresult_illegal_method_call
            (L"Changing DisableRedirectionLayer after calling XamlApplication.Start() is not supported.");
        }

        s_disableRedirectionLayer = value;
    }

    bool XamlConfig::EnableTouchpadAwareness()
    {
        return s_enableTouchpadAwareness;
    }

    void XamlConfig::EnableTouchpadAwareness(bool value)
    {
        if (s_isInitialized) [[unlikely]]
        {
            throw hresult_illegal_method_call
            (L"Changing EnableTouchpadAwareness after calling XamlApplication.Start() is not supported.");
        }

        s_enableTouchpadAwareness = value;
    }

    bool XamlConfig::DisableEarlyXamlShutdown()
    {
        return s_disableEarlyXamlShutdown;
    }

    void XamlConfig::DisableEarlyXamlShutdown(bool value)
    {
        s_disableEarlyXamlShutdown = value;
    }

    bool XamlConfig::EnableMsAppxWebProtocolSupport()
    {
        return s_enableMsAppxWebProtocolSupport;
    }

    void XamlConfig::EnableMsAppxWebProtocolSupport(bool value)
    {
        if (s_isInitialized) [[unlikely]]
        {
            throw hresult_illegal_method_call
            (L"Changing EnableMsAppxWebProtocolSupport after calling XamlApplication.Start() is not supported.");
        }

        s_enableMsAppxWebProtocolSupport = value && UrlmonCreateInstance;
    }

    bool XamlConfig::EnableArbitraryPathsInMsAppxWeb()
    {
        return s_enableArbitraryPathsInMsAppxWeb;
    }

    void XamlConfig::EnableArbitraryPathsInMsAppxWeb(bool value)
    {
        if (s_isInitialized) [[unlikely]]
        {
            throw hresult_illegal_method_call
            (L"Changing EnableArbitraryPathsInMsAppxWeb after calling XamlApplication.Start() is not supported.");
        }

        s_enableArbitraryPathsInMsAppxWeb = value && s_enableMsAppxWebProtocolSupport;
    }
}
