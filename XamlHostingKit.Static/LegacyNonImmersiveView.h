#pragma once

#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.ApplicationModel.Activation.h>

#pragma push_macro("WINRT_LEAN_AND_MEAN")
#undef WINRT_LEAN_AND_MEAN
#include <winrt/Windows.ApplicationModel.Core.h>
#pragma pop_macro("WINRT_LEAN_AND_MEAN")

namespace winrt::XamlHostingKit::implementation
{
    using namespace winrt::Windows::UI::Core;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::ApplicationModel::Core;
    using namespace winrt::Windows::ApplicationModel::Activation;

    struct LegacyNonImmersiveView : winrt::implements<LegacyNonImmersiveView, ICoreApplicationView, ICoreApplicationView2>
    {
    private:
        bool m_isMain { false };
        CoreWindow m_coreWindow { nullptr };
        winrt::com_ptr<::IUnknown> m_realView { nullptr };
        winrt::event<TypedEventHandler<CoreApplicationView, IActivatedEventArgs>> m_activatedEvent;

    public:
        LegacyNonImmersiveView() = default;
        LegacyNonImmersiveView(CoreWindow const& coreWindow, bool isMain = false, ::IUnknown* realView = nullptr);

        CoreWindow CoreWindow() const;
        bool IsMain() const;
        bool IsHosted() const;
        CoreDispatcher Dispatcher() const;

        winrt::event_token Activated(TypedEventHandler<CoreApplicationView, IActivatedEventArgs> const& handler);
        void Activated(winrt::event_token const& token) noexcept;

        int32_t query_interface_tearoff(winrt::guid const& id, void** object);
    };
}