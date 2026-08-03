#include "pch.h"
#include "LegacyNonImmersiveView.h"

namespace winrt::XamlHostingKit::implementation
{
    LegacyNonImmersiveView::LegacyNonImmersiveView(winrt::Windows::UI::Core::CoreWindow const& coreWindow, bool isMain, ::IUnknown* realView)
        : m_coreWindow(coreWindow),
          m_isMain(isMain)
    {
        m_realView.attach(realView);
    }

    bool LegacyNonImmersiveView::IsMain() const
    {
        return m_isMain;
    }

    bool LegacyNonImmersiveView::IsHosted() const
    {
        return false;
    }

    CoreWindow LegacyNonImmersiveView::CoreWindow() const
    {
        return m_coreWindow;
    }

    winrt::event_token LegacyNonImmersiveView::Activated(TypedEventHandler<CoreApplicationView, IActivatedEventArgs> const& handler)
    {
        return m_activatedEvent.add(handler);
    }

    void LegacyNonImmersiveView::Activated(winrt::event_token const& token) noexcept
    {
        m_activatedEvent.remove(token);
    }

    int32_t LegacyNonImmersiveView::query_interface_tearoff(winrt::guid const& id, void** object)
    {
        if (m_realView && id != winrt::guid_of<ICoreApplicationView>())
        {
            return m_realView.as(id, object);
        }

        return 0;
    }
}