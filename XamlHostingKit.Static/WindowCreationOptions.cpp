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

    std::int32_t WindowCreationOptions::Left() const
    {
        return m_left;
    }

    void WindowCreationOptions::Left(std::int32_t value)
    {
        m_left = value;
    }

    std::int32_t WindowCreationOptions::Top() const
    {
        return m_top;
    }

    void WindowCreationOptions::Top(std::int32_t value)
    {
        m_top = value;
    }

    std::int32_t WindowCreationOptions::Width() const
    {
        return m_width;
    }

    void WindowCreationOptions::Width(std::int32_t value)
    {
        m_width = value;
    }

    std::int32_t WindowCreationOptions::Height() const
    {
        return m_height;
    }

    void WindowCreationOptions::Height(std::int32_t value)
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
}
