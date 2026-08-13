#pragma once

#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <windows.ui.composition.interop.h>

#pragma push_macro("WINRT_LEAN_AND_MEAN")
#undef WINRT_LEAN_AND_MEAN
#include <winrt/Windows.ApplicationModel.Core.h>
#pragma pop_macro("WINRT_LEAN_AND_MEAN")

#include "XamlWindow.h"

namespace winrt::XamlHostingKit::implementation
{
	using namespace winrt::Windows::ApplicationModel::Core;
	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::UI::Composition;
	using namespace winrt::Windows::UI::Composition::Desktop;

	struct LegacyTitleBar : winrt::implements<LegacyTitleBar, ICoreApplicationViewTitleBar>
	{
	private:
		static const constexpr auto XHK_TITLEBAR_OBJECT_PROP = L"COMplicated.XamlHostingKit.TileBarObject";
		static const constexpr auto XHK_TITLEBAR_CAPTION_WIDTH = 188;

		bool m_extend{ false };
		bool m_isVisible{ false };
		HWND m_window{ 0 };
		Compositor m_compositor{ nullptr };
		DesktopWindowTarget m_target{ nullptr };
		ContainerVisual m_rootVisual{ nullptr };
		ContainerVisual m_caption{ nullptr };

		winrt::event<winrt::Windows::Foundation::TypedEventHandler<CoreApplicationViewTitleBar, IInspectable>> m_isVisibleChanged;
		winrt::event<winrt::Windows::Foundation::TypedEventHandler<CoreApplicationViewTitleBar, IInspectable>> m_layoutMetricsChanged;

		static LRESULT CALLBACK XamlWindowSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR idSubclass, DWORD_PTR dwRefData);
		static ContainerVisual CreateCaptionButton(Compositor const& compositor, CompositionGeometry const& geometry, int index);

	public:
		LegacyTitleBar() = default;
		LegacyTitleBar(HWND window);

		bool ExtendViewIntoTitleBar() const;
		void ExtendViewIntoTitleBar(bool const& value);

		float Height() const;
		bool IsVisible() const;
		float SystemOverlayLeftInset() const;
		float SystemOverlayRightInset() const;

		winrt::event_token IsVisibleChanged(TypedEventHandler<CoreApplicationViewTitleBar, IInspectable> const& handler);
		void IsVisibleChanged(winrt::event_token const& token) noexcept;
		winrt::event_token LayoutMetricsChanged(TypedEventHandler<CoreApplicationViewTitleBar, IInspectable> const& handler);
		void LayoutMetricsChanged(winrt::event_token const& token) noexcept;

	};
}
