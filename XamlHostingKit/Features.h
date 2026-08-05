#pragma once

#include <winrt/Windows.Foundation.Metadata.h>
#include "Helpers.h"

namespace Features
{
    using namespace winrt::Windows::Foundation::Metadata;

    inline static const bool IsXamlRootAvailable = ApiInformation::IsTypePresent(L"Windows.UI.Xaml.XamlRoot");
    inline static const bool IsDispatcherQueueAvailable = ApiInformation::IsTypePresent(L"Windows.System.DispatcherQueue");
    inline static const bool IsSetSynchronizationInfoAvailable =
        ([]() -> bool
        {
            using namespace winrt::XamlHostingKit;

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