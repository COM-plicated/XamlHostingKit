#pragma once
#include "TitleBar.g.h"
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <windows.ui.composition.interop.h>

namespace winrt::XamlHostingKit
{
	using namespace winrt::Windows::UI::Composition;

	struct TitleBarCaptionButton
	{
		ContainerVisual m_container{ nullptr };
		SpriteVisual m_background{ nullptr };
		ShapeVisual m_shape{ nullptr };
		ShapeVisual m_shape_ext{ nullptr };

		void Update(CompositionColorBrush const& foreground, CompositionColorBrush const& background = nullptr);
	};
}

namespace winrt::XamlHostingKit::implementation
{
	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::UI;
	using namespace winrt::Windows::UI::Core;
	using namespace winrt::Windows::UI::Composition;
	using namespace winrt::Windows::UI::Composition::Desktop;

	struct TitleBar : TitleBarT<TitleBar>
	{
	private:
		static const constexpr auto XHK_TITLEBAR_OBJECT_PROP = L"COMplicated.XamlHostingKit.TileBarObject";
		static const constexpr auto XHK_TITLEBAR_CAPTION_WIDTH = 188.0f;
		static const constexpr auto XHK_TITLEBAR_CAPTION_BUTTON_WIDTH = 46.0f;
		static const constexpr auto XHK_TITLEBAR_CAPTION_BUTTON_HEIGHT = 32.0f;

		static const constexpr auto XHK_TITLEBAR_CAPTION_OTHER_BACKGROUND_LIGHT = Color({ 0, 0, 0, 0 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_OTHER_BACKGROUND_DARK = Color({ 0, 0, 0, 0 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_OTHER_HOVER_BACKGROUND_LIGHT = Color({ 255, 233, 233, 233 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_OTHER_HOVER_BACKGROUND_DARK = Color({ 255, 45, 45, 45 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_OTHER_ACTIVE_BACKGROUND_LIGHT = Color({ 255, 237, 237, 237 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_OTHER_ACTIVE_BACKGROUND_DARK = Color({ 255, 41, 41, 41 });

		static const constexpr auto XHK_TITLEBAR_CAPTION_CLOSE_BACKGROUND_LIGHT = Color({ 0, 0, 0, 0 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_CLOSE_BACKGROUND_DARK = Color({ 0, 0, 0, 0 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_CLOSE_HOVER_BACKGROUND_LIGHT = Color({ 255, 196, 43, 28 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_CLOSE_HOVER_BACKGROUND_DARK = Color({ 255, 196, 43, 28 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_CLOSE_ACTIVE_BACKGROUND_LIGHT = Color({ 255, 200, 60, 49 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_CLOSE_ACTIVE_BACKGROUND_DARK = Color({ 255, 179, 39, 28 });
		
		static const constexpr auto XHK_TITLEBAR_CAPTION_FOREGROUND_LIGHT = Color({ 255, 25, 25, 25 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_FOREGROUND_DARK = Color({ 255, 255, 255, 255 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_HOVER_FOREGROUND_LIGHT = Color({ 255, 25, 25, 25 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_HOVER_FOREGROUND_DARK = Color({ 255, 255, 255, 255 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_ACTIVE_FOREGROUND_LIGHT = Color({ 255, 96, 96, 96 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_ACTIVE_FOREGROUND_DARK = Color({ 255, 207, 207, 207 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_INACTIVE_FOREGROUND_LIGHT = Color({ 255, 155, 155, 155 });
		static const constexpr auto XHK_TITLEBAR_CAPTION_INACTIVE_FOREGROUND_DARK = Color({ 255, 113, 113, 113 });

		bool m_extend{ false };
		bool m_isVisible{ false };
		bool m_isActive{ false };
		HWND m_xamlWindow{ nullptr };
		HWND m_coreWindow{ nullptr };

		Compositor m_compositor{ nullptr };
		DesktopWindowTarget m_target{ nullptr };
		ContainerVisual m_rootVisual{ nullptr };
		ContainerVisual m_caption{ nullptr };
		CoreDispatcher m_dispatcher{ nullptr };

		CompositionColorBrush m_captionOtherBackground{ nullptr };
		CompositionColorBrush m_captionCloseBackground{ nullptr };

		CompositionColorBrush m_captionOtherHoverBackground{ nullptr };
		CompositionColorBrush m_captionCloseHoverBackground{ nullptr };

		CompositionColorBrush m_captionOtherActiveBackground{ nullptr };
		CompositionColorBrush m_captionCloseActiveBackground{ nullptr };
		
		CompositionColorBrush m_captionForeground{ nullptr };
		CompositionColorBrush m_captionHoverForeground{ nullptr };
		CompositionColorBrush m_captionActiveForeground{ nullptr };;
		CompositionColorBrush m_captionInactiveForeground{ nullptr };;

		winrt::XamlHostingKit::TitleBarCaptionButton m_captionClose{ nullptr };
		winrt::XamlHostingKit::TitleBarCaptionButton m_captionMaximize{ nullptr };
		winrt::XamlHostingKit::TitleBarCaptionButton m_captionMinimize{ nullptr };

		winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::XamlHostingKit::TitleBar, IInspectable>> m_isVisibleChanged;
		winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::XamlHostingKit::TitleBar, IInspectable>> m_layoutMetricsChanged;

		void UpdateCaptionColors();

		static LRESULT CALLBACK XamlWindowSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR idSubclass, DWORD_PTR dwRefData);
		static LRESULT CALLBACK CoreWindowSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR idSubclass, DWORD_PTR dwRefData);
		static winrt::XamlHostingKit::TitleBarCaptionButton CreateCaptionButton(Compositor const& compositor, std::vector<CompositionGeometry> const& geometry, std::vector<CompositionGeometry> const& geometry_ext, int index);

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
