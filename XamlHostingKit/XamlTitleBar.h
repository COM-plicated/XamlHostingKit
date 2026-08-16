#pragma once
#include "XamlTitleBar.g.h"
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <windows.ui.composition.interop.h>
#include <dcomp.h>
#include <d3d11.h>

//#define TITLEBAR_USE_VISUALS

namespace winrt::XamlHostingKit
{
	using namespace winrt::Windows::UI::Composition;

	struct TitleBarCaptionButton
	{
#ifdef TITLEBAR_USE_VISUALS
		ContainerVisual m_container{ nullptr };
		SpriteVisual m_background{ nullptr };
		ShapeVisual m_shape{ nullptr };
		ShapeVisual m_shape_ext{ nullptr };

		void Update(CompositionColorBrush const& foreground, CompositionColorBrush const& background = nullptr);
#endif
	};

	enum class TitleBarCaptionButtonType
	{
		NONE = 0,
		CLOSE = 20,
		MAXIMIZE = 9,
		MINIMIZE = 8
	};

	enum class TitleBarCaptionButtonState
	{
		NORMAL = 0,
		HOVER = 1,
		ACTIVE = 2
	};
}

namespace winrt::XamlHostingKit::implementation
{
	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::UI;
	using namespace winrt::Windows::UI::Core;
	using namespace winrt::Windows::UI::Composition;
	using namespace winrt::Windows::UI::Composition::Desktop;

	struct XamlTitleBar : XamlTitleBarT<XamlTitleBar>
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
		bool m_isMaximized{ false };
		HWND m_xamlWindow{ nullptr };
		HWND m_coreWindow{ nullptr };
		CoreDispatcher m_dispatcher{ nullptr };
		Compositor m_compositor{ nullptr };

#ifdef TITLEBAR_USE_VISUALS
		DesktopWindowTarget m_target{ nullptr };
		ContainerVisual m_rootVisual{ nullptr };
		ContainerVisual m_caption{ nullptr };

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
#else
		winrt::com_ptr<ID3D11Device> m_d3d11Device{ nullptr };
		winrt::com_ptr<ID2D1Device> m_d2d1Device{ nullptr };
		winrt::com_ptr<IDCompositionDesktopDevice> m_dcompDevice{ nullptr };
		winrt::com_ptr<IDCompositionTarget> m_target{ nullptr };
		winrt::com_ptr<IDCompositionVisual2> m_rootVisual{ nullptr };
		winrt::com_ptr<IDCompositionVisual2> m_caption{ nullptr };
		winrt::com_ptr<IDCompositionSurface> m_captionSurface{ nullptr };
		winrt::com_ptr<IDCompositionSurfaceFactory> m_surfaceFactory{ nullptr };

		D2D1::ColorF m_captionOtherBackground{ 0, 1.0f };
		D2D1::ColorF m_captionCloseBackground{ 0, 1.0f };

		D2D1::ColorF m_captionOtherHoverBackground{ 0, 1.0f };
		D2D1::ColorF m_captionCloseHoverBackground{ 0, 1.0f };

		D2D1::ColorF m_captionOtherActiveBackground{ 0, 1.0f };
		D2D1::ColorF m_captionCloseActiveBackground{ 0, 1.0f };

		D2D1::ColorF m_captionForeground{ 0, 1.0f };
		D2D1::ColorF m_captionHoverForeground{ 0, 1.0f };
		D2D1::ColorF m_captionActiveForeground{ 0, 1.0f };
		D2D1::ColorF m_captionInactiveForeground{ 0, 1.0f };
#endif

		winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::XamlHostingKit::XamlTitleBar, IInspectable>> m_isVisibleChanged;
		winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::XamlHostingKit::XamlTitleBar, IInspectable>> m_layoutMetricsChanged;

		void UpdateCaptionColors();
#ifndef TITLEBAR_USE_VISUALS
		void CreateCompositionDevice();
		void CreateCaptionSurface(float scale);
		void DrawCaption(float scale, winrt::XamlHostingKit::TitleBarCaptionButtonType const& buttonType, winrt::XamlHostingKit::TitleBarCaptionButtonState const& buttonState);
		void CaptionButtonColor(winrt::XamlHostingKit::TitleBarCaptionButtonType const& buttonType, winrt::XamlHostingKit::TitleBarCaptionButtonState const& buttonState, D2D1::ColorF* bgColor, D2D1::ColorF* stColor);
		void CommitComposition();
		void ReleaseResources();
#endif

		static LRESULT CALLBACK XamlWindowSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR idSubclass, DWORD_PTR dwRefData);
		static LRESULT CALLBACK CoreWindowSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR idSubclass, DWORD_PTR dwRefData);
#ifdef TITLEBAR_USE_VISUALS
		static winrt::XamlHostingKit::TitleBarCaptionButton CreateCaptionButton(Compositor const& compositor, std::vector<CompositionGeometry> const& geometry, std::vector<CompositionGeometry> const& geometry_ext, int index);
#else
		static D2D1::ColorF ToColorF(Color const& color);
#endif

	public:
		XamlTitleBar() = default;
		XamlTitleBar(HWND const& xamlWindow, HWND const& coreWindow, Compositor const& compositor, CoreDispatcher const& dispatcher);

		bool ExtendViewIntoTitleBar() const;
		void ExtendViewIntoTitleBar(bool const& value);

		float Height() const;
		bool IsVisible() const;
		float SystemOverlayLeftInset() const;
		float SystemOverlayRightInset() const;

		winrt::event_token IsVisibleChanged(TypedEventHandler<winrt::XamlHostingKit::XamlTitleBar, IInspectable> const& handler);
		void IsVisibleChanged(winrt::event_token const& token) noexcept;
		winrt::event_token LayoutMetricsChanged(TypedEventHandler<winrt::XamlHostingKit::XamlTitleBar, IInspectable> const& handler);
		void LayoutMetricsChanged(winrt::event_token const& token) noexcept;

	};
}
