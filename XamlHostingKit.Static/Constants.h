#pragma once

#include "Helpers.h"
#include "XamlConfig.h"

#define Constants xhk_Constants
namespace Constants
{
    namespace
    {
        using namespace winrt::XamlHostingKit;
        using namespace winrt::XamlHostingKit::implementation;
        using XamlConfig = winrt::XamlHostingKit::implementation::XamlConfig;
    }

    // BUILD CONSTANTS

    //static const constexpr auto MrmAppInitMinimumBuild = 18362u;
    static const constexpr auto MrmAppInitMinimumBuild = 22000u;
    
    
    // RUNTIME CONSTANTS

    static const bool CanUseMrmAppInit = (!Helpers::CurrentPackageFamilyName.empty()) || (Helpers::OSBuild >= Constants::MrmAppInitMinimumBuild);
    static const bool ShouldUseMrmAppInit = CanUseMrmAppInit && !XamlConfig::s_alwaysUseCustomPriLoader;
}