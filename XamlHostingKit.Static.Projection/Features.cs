using System;
using System.Diagnostics.CodeAnalysis;

namespace XamlHostingKit.Static.Projection
{
    internal static class Features
    {
        private const string UseActivationHooksFeature = "XamlHostingKit.UseActivationHooks";

        [FeatureSwitchDefinition(UseActivationHooksFeature)]
        public static bool UseActivationHooks =>
            AppContext.TryGetSwitch(UseActivationHooksFeature, out bool enabled) ? enabled : true;
    }
}
