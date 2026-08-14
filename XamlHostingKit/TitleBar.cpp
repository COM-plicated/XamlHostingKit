#include "pch.h"
#include "TitleBar.h"
#include "TitleBar.g.cpp"
#include "Helpers.h"
#include <windowsx.h>
#include <dwmapi.h>
#include <shellapi.h>

namespace winrt::XamlHostingKit
{
	void winrt::XamlHostingKit::TitleBarCaptionButton::Update(CompositionColorBrush const& foreground, CompositionColorBrush const& background)
	{
		for (const auto& part : m_shape.Shapes())
			part.as<CompositionSpriteShape>().StrokeBrush(foreground);

		if (m_shape_ext)
			for (const auto& part : m_shape_ext.Shapes())
				part.as<CompositionSpriteShape>().StrokeBrush(foreground);

		if (background)
			m_background.Brush(background);
	}
}

namespace winrt::XamlHostingKit::implementation
{

	TitleBar::TitleBar(HWND const& xamlWindow, HWND const& coreWindow, CoreDispatcher const& dispatcher)
		: m_xamlWindow(xamlWindow), m_coreWindow(coreWindow), m_dispatcher(dispatcher)
	{
		m_isVisible = true;

		namespace abi = ABI::Windows::UI::Composition::Desktop;
		m_compositor = Compositor();

		auto interop = m_compositor.as<abi::ICompositorDesktopInterop>();

		interop->CreateDesktopWindowTarget(m_xamlWindow, true, reinterpret_cast<abi::IDesktopWindowTarget**>(winrt::put_abi(m_target)));

		m_rootVisual = m_compositor.CreateContainerVisual();
		m_rootVisual.RelativeSizeAdjustment({ 1.0f, 1.0f });
		m_target.Root(m_rootVisual);

		m_caption = m_compositor.CreateContainerVisual();
		m_caption.Size({ XHK_TITLEBAR_CAPTION_WIDTH, static_cast<float>(Helpers::GetCaptionSize(m_xamlWindow)) });
		m_caption.Offset({ -XHK_TITLEBAR_CAPTION_WIDTH, static_cast<float>(Helpers::GetTopBorderSize(m_xamlWindow)), 0.0f });
		m_caption.RelativeOffsetAdjustment({ 1.0, 0.0f, 0.0f });
		m_rootVisual.Children().InsertAtTop(m_caption);

		UpdateCaptionColors();

		auto captionBg = m_compositor.CreateSpriteVisual();
		captionBg.RelativeSizeAdjustment({ 1.0f, 1.0f });
		auto brush = m_compositor.CreateColorBrush({ 0, 0, 0, 0 });
		captionBg.Brush(brush);

		auto close1 = m_compositor.CreateLineGeometry();
		close1.Start({ 0.0f, 0.0f });
		close1.End({ 10.0f, 10.0f });
		auto close2 = m_compositor.CreateLineGeometry();
		close2.Start({ 0.0f, 10.0f });
		close2.End({ 10.0f, 0.0f });

		auto max = m_compositor.CreateRoundedRectangleGeometry();
		max.Size({ 9.0f, 9.0f });
		max.Offset({ 0.5f, 0.5f });
		max.CornerRadius({ 1.0f, 1.0f });

		auto restore = m_compositor.CreateRoundedRectangleGeometry();
		restore.Size({ 7.0f, 7.0f });
		restore.Offset({ 0.5f, 2.5f });
		restore.CornerRadius({ 1.0f, 1.0f });

		auto restore_ext_1 = m_compositor.CreateLineGeometry();
		restore_ext_1.Start({ 2.5f, 0.5f });
		restore_ext_1.End({ 8.0f, 0.5f });

		auto restore_ext_2 = m_compositor.CreateLineGeometry();
		restore_ext_2.Start({ 8.0f, 0.5f });
		restore_ext_2.End({ 9.5f, 2.0f });

		auto restore_ext_3 = m_compositor.CreateLineGeometry();
		restore_ext_3.Start({ 9.5f, 2.0f });
		restore_ext_3.End({ 9.5f, 7.5f });

		/*auto restore_ext = m_compositor.CreateRoundedRectangleGeometry();
		restore_ext.Size({ 8.0f, 8.0f });
		restore_ext.Offset({ 1.5f, 0.5f });
		restore_ext.CornerRadius({ 2.0f, 2.0f });*/

		auto min = m_compositor.CreateLineGeometry();
		min.Start({ 0.0f, 4.5f });
		min.End({ 10.0f, 4.5f });

		m_caption.Children().InsertAtBottom(captionBg);
		m_caption.Children().InsertAtTop((m_captionClose = CreateCaptionButton(m_compositor, { close1, close2 }, { }, 1)).m_container);
		m_caption.Children().InsertAtTop((m_captionMaximize = CreateCaptionButton(m_compositor, { max }, { restore, restore_ext_1, restore_ext_2, restore_ext_3 }, 2)).m_container);
		m_caption.Children().InsertAtTop((m_captionMinimize = CreateCaptionButton(m_compositor, { min }, { }, 3)).m_container);

		m_caption.IsVisible(m_extend);

		SetPropW(m_xamlWindow, XHK_TITLEBAR_OBJECT_PROP, this);
		SetPropW(m_coreWindow, XHK_TITLEBAR_OBJECT_PROP, this);

		SetWindowSubclass(m_xamlWindow, XamlWindowSubClassProc, 1, NULL);
		SetWindowSubclass(m_coreWindow, CoreWindowSubClassProc, 1, NULL);
	}

	bool TitleBar::ExtendViewIntoTitleBar() const {
		return m_extend;
	}

	void TitleBar::ExtendViewIntoTitleBar(bool const& value) {
		m_extend = value;
		m_caption.IsVisible(value);
		SetWindowPos(m_xamlWindow, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		if (m_layoutMetricsChanged) {
			m_dispatcher.RunAsync(CoreDispatcherPriority::Normal, [=]()
				{
					m_layoutMetricsChanged(*this, nullptr);
				});
		}
	}

	float TitleBar::Height() const
	{
		return static_cast<float>(Helpers::GetCaptionSize(m_xamlWindow));
	}

	bool TitleBar::IsVisible() const
	{
		return m_isVisible;
	}

	float TitleBar::SystemOverlayLeftInset() const
	{
		return 0;
	}

	float TitleBar::SystemOverlayRightInset() const
	{
		return m_extend ? XHK_TITLEBAR_CAPTION_WIDTH : 0;
	}

	winrt::event_token TitleBar::IsVisibleChanged(TypedEventHandler<winrt::XamlHostingKit::TitleBar, IInspectable> const& handler)
	{
		return m_isVisibleChanged.add(handler);
	}

	void TitleBar::IsVisibleChanged(winrt::event_token const& token) noexcept
	{
		m_isVisibleChanged.remove(token);
	}

	winrt::event_token TitleBar::LayoutMetricsChanged(TypedEventHandler<winrt::XamlHostingKit::TitleBar, IInspectable> const& handler)
	{
		return m_layoutMetricsChanged.add(handler);
	}

	void TitleBar::LayoutMetricsChanged(winrt::event_token const& token) noexcept
	{
		m_layoutMetricsChanged.remove(token);
	}

	void TitleBar::UpdateCaptionColors()
	{
		auto dark = Helpers::ShouldAppsUseDarkMode();

		m_captionOtherBackground = m_compositor.CreateColorBrush(dark ? XHK_TITLEBAR_CAPTION_OTHER_BACKGROUND_DARK : XHK_TITLEBAR_CAPTION_OTHER_BACKGROUND_LIGHT);
		m_captionOtherHoverBackground = m_compositor.CreateColorBrush(dark ? XHK_TITLEBAR_CAPTION_OTHER_HOVER_BACKGROUND_DARK : XHK_TITLEBAR_CAPTION_OTHER_HOVER_BACKGROUND_LIGHT);
		m_captionOtherActiveBackground = m_compositor.CreateColorBrush(dark ? XHK_TITLEBAR_CAPTION_OTHER_ACTIVE_BACKGROUND_DARK : XHK_TITLEBAR_CAPTION_OTHER_ACTIVE_BACKGROUND_LIGHT);

		m_captionCloseBackground = m_compositor.CreateColorBrush(dark ? XHK_TITLEBAR_CAPTION_CLOSE_BACKGROUND_DARK : XHK_TITLEBAR_CAPTION_CLOSE_BACKGROUND_LIGHT);
		m_captionCloseHoverBackground = m_compositor.CreateColorBrush(dark ? XHK_TITLEBAR_CAPTION_CLOSE_HOVER_BACKGROUND_DARK : XHK_TITLEBAR_CAPTION_CLOSE_HOVER_BACKGROUND_LIGHT);
		m_captionCloseActiveBackground = m_compositor.CreateColorBrush(dark ? XHK_TITLEBAR_CAPTION_CLOSE_ACTIVE_BACKGROUND_DARK : XHK_TITLEBAR_CAPTION_CLOSE_ACTIVE_BACKGROUND_LIGHT);

		m_captionForeground = m_compositor.CreateColorBrush(dark ? XHK_TITLEBAR_CAPTION_FOREGROUND_DARK : XHK_TITLEBAR_CAPTION_FOREGROUND_LIGHT);
		m_captionHoverForeground = m_compositor.CreateColorBrush(dark ? XHK_TITLEBAR_CAPTION_HOVER_FOREGROUND_DARK : XHK_TITLEBAR_CAPTION_HOVER_FOREGROUND_LIGHT);
		m_captionActiveForeground = m_compositor.CreateColorBrush(dark ? XHK_TITLEBAR_CAPTION_ACTIVE_FOREGROUND_DARK : XHK_TITLEBAR_CAPTION_ACTIVE_FOREGROUND_LIGHT);
		m_captionInactiveForeground = m_compositor.CreateColorBrush(dark ? XHK_TITLEBAR_CAPTION_INACTIVE_FOREGROUND_DARK : XHK_TITLEBAR_CAPTION_INACTIVE_FOREGROUND_LIGHT);
	}

	LRESULT TitleBar::XamlWindowSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
	{
		auto _this = reinterpret_cast<TitleBar*>(GetPropW(hwnd, XHK_TITLEBAR_OBJECT_PROP));

		if (msg == WM_DESTROY)
		{
			RemovePropW(hwnd, XHK_TITLEBAR_OBJECT_PROP);
		}
		else if (_this)
		{
			if (msg == WM_SIZE)
			{
				auto scaling = Helpers::GetDpiScaleForWindow(hwnd);

				auto scaledCaptionSize = XHK_TITLEBAR_CAPTION_WIDTH * scaling;
				_this->m_caption.Offset({ -scaledCaptionSize, static_cast<float>(Helpers::GetTopBorderSize(hwnd)), 0 });
				_this->m_caption.Size({ scaledCaptionSize, static_cast<float>(Helpers::GetCaptionSize(hwnd)) });

				auto scaledButtonSize = XHK_TITLEBAR_CAPTION_BUTTON_WIDTH * scaling;
				_this->m_captionClose.m_container.Offset({ -scaledButtonSize , 0.0f, 0.0f });
				_this->m_captionClose.m_container.Scale({ scaling, scaling, 1.0f });

				_this->m_captionMaximize.m_container.Offset({ -scaledButtonSize * 2 , 0.0f, 0.0f });
				_this->m_captionMaximize.m_container.Scale({ scaling, scaling, 1.0f });

				_this->m_captionMinimize.m_container.Offset({ -scaledButtonSize * 3 , 0.0f, 0.0f });
				_this->m_captionMinimize.m_container.Scale({ scaling, scaling, 1.0f });

				_this->m_captionMaximize.m_shape.IsVisible(wParam != SIZE_MAXIMIZED);
				_this->m_captionMaximize.m_shape_ext.IsVisible(wParam == SIZE_MAXIMIZED);

			}
			else if (msg == WM_NCHITTEST && _this->m_extend)
			{
				auto x = GET_X_LPARAM(lParam);
				auto y = GET_Y_LPARAM(lParam);
				auto ret = DefSubclassProc(hwnd, msg, wParam, lParam);

				if (ret == HTCLIENT)
				{
					RECT rc;
					GetWindowRect(hwnd, &rc);
					auto dpi = Helpers::GetDpiForWindow(hwnd);
					auto border = Helpers::GetSystemMetricsForDpi(SM_CXFRAME, dpi) + Helpers::GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
					auto caption = Helpers::GetTopBorderSize(hwnd) + Helpers::GetCaptionSize(hwnd);

					if (y > rc.top && y < rc.top + caption)
					{
						auto buttonSize = XHK_TITLEBAR_CAPTION_BUTTON_WIDTH * Helpers::GetDpiScaleForWindow(hwnd);
						auto relative = rc.right - border;
						if (x < relative && x >= relative - buttonSize)
							return HTCLOSE;
						else if (x < relative - buttonSize && x >= relative - buttonSize * 2)
							return HTMAXBUTTON;
						else  if (x < relative - buttonSize * 2 && x >= relative - buttonSize * 3)
							return HTMINBUTTON;
					}

					if (y < rc.top + border)
					{
						if (x < rc.left + border * 2)
							return HTTOPLEFT;
						else if (x > rc.right - border * 2)
							return HTTOPRIGHT;
						else
							return HTTOP;
					}
					else
						return HTCAPTION;
				}
				return ret;
			}
			else if (msg == WM_NCCALCSIZE)
			{
				if (wParam && _this->m_extend) {
					auto dpi = Helpers::GetDpiForWindow(hwnd);
					auto border = Helpers::GetSystemMetricsForDpi(SM_CXFRAME, dpi) + Helpers::GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
					NCCALCSIZE_PARAMS* pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
					pncsp->rgrc[0].left += border;
					pncsp->rgrc[0].top += 0;
					pncsp->rgrc[0].right -= border;
					pncsp->rgrc[0].bottom -= border;
					return 0;
				}
			}
			else if (msg == WM_WINDOWPOSCHANGING)
			{
				WINDOWPOS* pos = reinterpret_cast<WINDOWPOS*>(lParam);
				if (pos->flags & SWP_FRAMECHANGED)
				{
					MARGINS margins{};
					if (_this->m_extend)
						margins.cyTopHeight = Helpers::GetCaptionSize(hwnd) + Helpers::GetTopBorderSize(hwnd);
					else
						margins.cyTopHeight = 0;
					DwmExtendFrameIntoClientArea(hwnd, &margins);
				}
			}
			else if (msg == WM_NCLBUTTONDOWN || msg == WM_NCLBUTTONDBLCLK)
			{
				switch (wParam)
				{
				case HTCLOSE:
					_this->m_captionClose.Update(_this->m_captionActiveForeground, _this->m_captionCloseActiveBackground);
					break;
				case HTMAXBUTTON:
					_this->m_captionMaximize.Update(_this->m_captionActiveForeground, _this->m_captionOtherActiveBackground);
					break;
				case HTMINBUTTON:
					_this->m_captionMinimize.Update(_this->m_captionActiveForeground, _this->m_captionOtherActiveBackground);
					break;
				}
			}
			else if (msg == WM_NCLBUTTONUP)
			{
				switch (wParam)
				{
				case HTCLOSE:
					_this->m_captionClose.Update(_this->m_captionHoverForeground, _this->m_captionCloseHoverBackground);
					break;
				case HTMAXBUTTON:
					_this->m_captionMaximize.Update(_this->m_captionHoverForeground, _this->m_captionOtherHoverBackground);
					break;
				case HTMINBUTTON:
					_this->m_captionMinimize.Update(_this->m_captionHoverForeground, _this->m_captionOtherHoverBackground);
					break;
				}
			}
			else if (msg == WM_NCMOUSEMOVE)
			{
				switch (wParam)
				{
				case HTCLOSE:
					_this->m_captionClose.Update(_this->m_captionHoverForeground, _this->m_captionCloseHoverBackground);
					_this->m_captionMaximize.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionOtherBackground);
					_this->m_captionMinimize.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionOtherBackground);
					break;
				case HTMAXBUTTON:
					_this->m_captionClose.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionCloseBackground);
					_this->m_captionMaximize.Update(_this->m_captionHoverForeground, _this->m_captionOtherHoverBackground);
					_this->m_captionMinimize.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionOtherBackground);
					break;
				case HTMINBUTTON:
					_this->m_captionClose.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionCloseBackground);
					_this->m_captionMaximize.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionOtherBackground);
					_this->m_captionMinimize.Update(_this->m_captionHoverForeground, _this->m_captionOtherHoverBackground);
					break;
				default:
					_this->m_captionClose.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionCloseBackground);
					_this->m_captionMaximize.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionOtherBackground);
					_this->m_captionMinimize.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionOtherBackground);
					break;
				}
			}
			else if (msg == WM_NCMOUSELEAVE || msg == WM_MOUSELEAVE)
			{
				_this->m_captionClose.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionCloseBackground);
				_this->m_captionMaximize.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionOtherBackground);
				_this->m_captionMinimize.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionOtherBackground);
			}
			else if (msg == WM_NCACTIVATE)
			{
				if (_this->m_isActive = wParam)
				{
					_this->m_captionClose.Update(_this->m_captionForeground);
					_this->m_captionMaximize.Update(_this->m_captionForeground);
					_this->m_captionMinimize.Update(_this->m_captionForeground);
				}
				else
				{
					_this->m_captionClose.Update(_this->m_captionInactiveForeground);
					_this->m_captionMaximize.Update(_this->m_captionInactiveForeground);
					_this->m_captionMinimize.Update(_this->m_captionInactiveForeground);
				}
			}
			else if (msg == WM_SETTINGCHANGE)
			{
				_this->UpdateCaptionColors();
			}
		}

		return DefSubclassProc(hwnd, msg, wParam, lParam);
	}

	LRESULT TitleBar::CoreWindowSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
	{
		auto _this = reinterpret_cast<TitleBar*>(GetPropW(hwnd, XHK_TITLEBAR_OBJECT_PROP));

		if (msg == WM_DESTROY)
		{
			RemovePropW(hwnd, XHK_TITLEBAR_OBJECT_PROP);
		}
		else if (_this)
		{
			if (msg == WM_NCHITTEST && _this->m_extend)
			{
				auto x = GET_X_LPARAM(lParam);
				auto y = GET_Y_LPARAM(lParam);
				auto ret = DefSubclassProc(hwnd, msg, wParam, lParam);

				if (ret == HTCLIENT)
				{
					RECT rc;
					GetWindowRect(hwnd, &rc);
					auto fakeborder = std::ceil(1 * Helpers::GetDpiScaleForWindow(hwnd));
					if (y < rc.top + Helpers::GetCaptionSize(hwnd) + Helpers::GetTopBorderSize(hwnd) - fakeborder)
						return HTTRANSPARENT;
					else
						return ret;
				}
			}
		}

		return DefSubclassProc(hwnd, msg, wParam, lParam);
	}

	winrt::XamlHostingKit::TitleBarCaptionButton TitleBar::CreateCaptionButton(Compositor const& compositor, std::vector<CompositionGeometry> const& geometry, std::vector<CompositionGeometry> const& geometry_ext, int index)
	{
		TitleBarCaptionButton button = { };

		auto container = compositor.CreateContainerVisual();
		container.Size({ XHK_TITLEBAR_CAPTION_BUTTON_WIDTH, XHK_TITLEBAR_CAPTION_BUTTON_HEIGHT });
		container.Offset({ -XHK_TITLEBAR_CAPTION_BUTTON_WIDTH * index , 0.0f, 0.0f });
		//container.RelativeSizeAdjustment({ 0.0f, 1.0f });
		container.RelativeOffsetAdjustment({ 1.0f, 0.0f, 0.0f });

		auto background = compositor.CreateSpriteVisual();
		background.RelativeSizeAdjustment({ 1.0f, 1.0f });

		container.Children().InsertAtBottom(background);

		auto color = compositor.CreateColorBrush({ 255, 255, 255, 255 });

		auto shape = compositor.CreateShapeVisual();
		shape.Size({ 10.0f, 10.0f });
		shape.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0.0f });
		shape.AnchorPoint({ 0.5f, 0.5f });

		for (const auto& part : geometry) {
			auto sprite = compositor.CreateSpriteShape(part);
			sprite.StrokeBrush(color);
			sprite.StrokeThickness(1.0f);
			shape.Shapes().Append(sprite);
		}

		container.Children().InsertAtTop(shape);

		button.m_container = container;
		button.m_background = background;
		button.m_shape = shape;

		if (geometry_ext.size() > 0) {
			auto shape_ext = compositor.CreateShapeVisual();
			shape_ext.Size({ 10.0f, 10.0f });
			shape_ext.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0.0f });
			shape_ext.AnchorPoint({ 0.5f, 0.5f });

			for (const auto& part : geometry_ext) {
				auto sprite = compositor.CreateSpriteShape(part);
				sprite.StrokeBrush(color);
				sprite.StrokeThickness(1.0f);
				shape_ext.Shapes().Append(sprite);
			}

			container.Children().InsertAtTop(shape_ext);
			button.m_shape_ext = shape_ext;
		}

		return button;
	}

}
