#include "pch.h"
#include "WindowCreationOptions.h"
#include "WindowCreationOptions.g.cpp"

namespace winrt::XamlHostingKit::implementation
{
    hstring WindowCreationOptions::Title()
    {
        return m_title;
    }

    void WindowCreationOptions::Title(hstring const& value)
    {
        m_title = value;
    }

    Windows::Foundation::IReference<float> WindowCreationOptions::Left() const
    {
        return m_left;
    }

    void WindowCreationOptions::Left(Windows::Foundation::IReference<float> value)
    {
        m_left = value;
    }

    Windows::Foundation::IReference<float> WindowCreationOptions::Top() const
    {
        return m_top;
    }

    void WindowCreationOptions::Top(Windows::Foundation::IReference<float> value)
    {
        m_top = value;
    }

    Windows::Foundation::IReference<float> WindowCreationOptions::Width() const
    {
        return m_width;
    }

    void WindowCreationOptions::Width(Windows::Foundation::IReference<float> value)
    {
        m_width = value;
    }

    Windows::Foundation::IReference<float> WindowCreationOptions::Height() const
    {
        return m_height;
    }

    void WindowCreationOptions::Height(Windows::Foundation::IReference<float> value)
    {
        m_height = value;
    }

    WindowStyles WindowCreationOptions::Styles() const
    {
        return m_styles;
    }

    void WindowCreationOptions::Styles(WindowStyles value)
    {
        m_styles = value;
    }

    WindowExtendedStyles WindowCreationOptions::ExtendedStyles() const
    {
        return m_extendedStyles;
    }

    void WindowCreationOptions::ExtendedStyles(WindowExtendedStyles value)
    {
        m_extendedStyles = value;
    }

    Windows::Foundation::IReference<winrt::Windows::Graphics::DisplayId> WindowCreationOptions::MonitorHandle() const
    {
        return m_monitorHandle;
    }

    void WindowCreationOptions::MonitorHandle(Windows::Foundation::IReference<winrt::Windows::Graphics::DisplayId> value)
    {
        m_monitorHandle = value;
    }

    Windows::UI::WindowManagement::DisplayRegion WindowCreationOptions::DisplayRegion() const
    {
        return m_displayRegion;
    }

    void WindowCreationOptions::DisplayRegion(Windows::UI::WindowManagement::DisplayRegion value)
    {
        m_displayRegion = value;
    }
}
