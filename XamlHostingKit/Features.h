#pragma once

#include <winrt/Windows.Foundation.Metadata.h>

namespace Features
{
    using namespace winrt::Windows::Foundation::Metadata;

    inline static const bool IsXamlRootAvailable = ApiInformation::IsTypePresent(L"Windows.UI.Xaml.XamlRoot");
    inline static const bool IsDispatcherQueueSupported = ApiInformation::IsTypePresent(L"Windows.System.DispatcherQueue");
}