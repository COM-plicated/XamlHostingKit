#include "pch.h"
#include "LegacyNonImmersiveView.h"
#include "LegacyTitleBar.h"

namespace winrt::XamlHostingKit::implementation
{
    LegacyNonImmersiveView::LegacyNonImmersiveView(winrt::Windows::UI::Core::CoreWindow const& coreWindow, bool isMain, ::IUnknown* realView, HWND parent)
        : m_coreWindow(coreWindow),
          m_isMain(isMain)
    {
        m_realView.attach(realView);
        m_titleBar = winrt::make<LegacyTitleBar>(parent, coreWindow.Dispatcher())
            .as<winrt::Windows::ApplicationModel::Core::CoreApplicationViewTitleBar>();
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

    CoreDispatcher LegacyNonImmersiveView::Dispatcher() const
    {
        return m_coreWindow.Dispatcher();
    }

    bool LegacyNonImmersiveView::IsComponent() const
    {
        return false;
    }

    CoreApplicationViewTitleBar LegacyNonImmersiveView::TitleBar() const
    {
        return m_titleBar;
    }

    winrt::event_token LegacyNonImmersiveView::Activated(TypedEventHandler<CoreApplicationView, IActivatedEventArgs> const& handler)
    {
        return m_activatedEvent.add(handler);
    }

    void LegacyNonImmersiveView::Activated(winrt::event_token const& token) noexcept
    {
        m_activatedEvent.remove(token);
    }

    winrt::event_token LegacyNonImmersiveView::HostedViewClosing(TypedEventHandler<CoreApplicationView, HostedViewClosingEventArgs> const& handler)
    {
        return winrt::event_token();
    }

    void LegacyNonImmersiveView::HostedViewClosing(winrt::event_token const& token) noexcept
    {
    }

    int32_t LegacyNonImmersiveView::query_interface_tearoff(winrt::guid const& id, void** object)
    {
        if (m_realView &&
           (id != winrt::guid_of<ICoreApplicationView>() && id != winrt::guid_of<ICoreApplicationView2>() && id != winrt::guid_of<ICoreApplicationView3>()))
        {
            return m_realView.as(id, object);
        }

        return 0;
    }
}