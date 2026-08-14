#pragma once

#include <winrt/Windows.Foundation.Metadata.h>
#include "Helpers.h"

namespace Features
{
    using namespace winrt::Windows::Foundation::Metadata;

    inline static const bool IsApiInformationAvailable = winrt::try_get_activation_factory<ApiInformation>() != nullptr;
    inline static const bool IsXamlRootAvailable = IsApiInformationAvailable && ApiInformation::IsTypePresent(L"Windows.UI.Xaml.XamlRoot");
    inline static const bool IsDispatcherQueueAvailable = IsApiInformationAvailable && ApiInformation::IsTypePresent(L"Windows.System.DispatcherQueue");
    inline static const bool IsDesktopWindowTargetAvailable = ApiInformation::IsTypePresent(L"Windows.UI.Composition.Desktop.DesktopWindowTarget");

    inline static const bool IsSetSynchronizationInfoAvailable =
        ([]() -> bool
        {
            using namespace winrt::XamlHostingKit;

            if (!IsApiInformationAvailable) [[unlikely]]
                return false;

            if (!NtUserGetResizeDCompositionSynchronizationObject) [[unlikely]]
                return false;

            if (ApiInformation::IsTypePresent(L"Windows.UI.Composition.Core.CompositorController"))
                return true;

            IMAGE_THUNK_DATA* thunk;
            return SUCCEEDED(Helpers::XWineGetImport(
                XAMLModule.get(),
                Win32UModule,
                "NtDCompositionCommitSynchronizationObject",
                &thunk)) && thunk;
        })();
}