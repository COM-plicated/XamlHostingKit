#include "pch.h"
#include "XamlConfig.h"
#include "XamlConfig.g.cpp"

namespace winrt::XamlHostingKit::implementation
{
    bool XamlConfig::s_isInitialized = false;
    bool XamlConfig::s_enableWebView = true;
    bool XamlConfig::s_disableRedirectionLayer = true;

    bool XamlConfig::EnableWebView()
    {
        return s_enableWebView;
    }

    void XamlConfig::EnableWebView(bool value)
    {
        if (s_isInitialized)
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
        if (s_isInitialized)
        {
            throw hresult_illegal_method_call
            (L"Changing DisableRedirectionLayer after calling XamlApplication.Start() is not supported.");
        }

        s_disableRedirectionLayer = value;
    }
}
