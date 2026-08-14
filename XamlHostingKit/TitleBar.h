#pragma once
#include "TitleBar.g.h"
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <windows.ui.composition.interop.h>

namespace winrt::XamlHostingKit::implementation
{
	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::UI::Core;
	using namespace winrt::Windows::UI::Composition;
	using namespace winrt::Windows::UI::Composition::Desktop;

	struct TitleBar : TitleBarT<TitleBar>
	{
	private:
		static const constexpr auto XHK_TITLEBAR_OBJECT_PROP = L"COMplicated.XamlHostingKit.TileBarObject";
		static const constexpr auto XHK_TITLEBAR_CAPTION_WIDTH = 188.0f;
		static const constexpr auto XHK_TITLEBAR_CAPTION_BUTTON_WIDTH = 46.0f;

		bool m_extend{ false };
		bool m_isVisible{ false };
		HWND m_xamlWindow{ nullptr };
		HWND m_coreWindow{ nullptr };
		Compositor m_compositor{ nullptr };
		DesktopWindowTarget m_target{ nullptr };
		ContainerVisual m_rootVisual{ nullptr };
		ContainerVisual m_caption{ nullptr };
		CoreDispatcher m_dispatcher{ nullptr };
		ContainerVisual m_captionClose{ nullptr };
		ContainerVisual m_captionMaximize{ nullptr };
		ContainerVisual m_captionMinimize{ nullptr };

		winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::XamlHostingKit::TitleBar, IInspectable>> m_isVisibleChanged;
		winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::XamlHostingKit::TitleBar, IInspectable>> m_layoutMetricsChanged;

		static LRESULT CALLBACK XamlWindowSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR idSubclass, DWORD_PTR dwRefData);
		static LRESULT CALLBACK CoreWindowSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR idSubclass, DWORD_PTR dwRefData);
		static ContainerVisual CreateCaptionButton(Compositor const& compositor, CompositionGeometry const& geometry, int index);

	public:
		TitleBar() = default;
		TitleBar(HWND const& xamlWindow, HWND const& coreWindow, CoreDispatcher const& dispatcher);

		bool ExtendViewIntoTitleBar() const;
		void ExtendViewIntoTitleBar(bool const& value);

		float Height() const;
		bool IsVisible() const;
		float SystemOverlayLeftInset() const;
		float SystemOverlayRightInset() const;

		winrt::event_token IsVisibleChanged(TypedEventHandler<winrt::XamlHostingKit::TitleBar, IInspectable> const& handler);
		void IsVisibleChanged(winrt::event_token const& token) noexcept;
		winrt::event_token LayoutMetricsChanged(TypedEventHandler<winrt::XamlHostingKit::TitleBar, IInspectable> const& handler);
		void LayoutMetricsChanged(winrt::event_token const& token) noexcept;

	};
}
