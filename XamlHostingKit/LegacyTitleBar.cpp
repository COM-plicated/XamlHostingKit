#include "pch.h"
#include "LegacyTitleBar.h"
#include <windowsx.h>
#include <dwmapi.h>
#include <shellapi.h>

namespace winrt::XamlHostingKit::implementation
{

	LegacyTitleBar::LegacyTitleBar(HWND window)
		: m_window(window)
	{
		m_isVisible = true;

		namespace abi = ABI::Windows::UI::Composition::Desktop;
		m_compositor = Compositor();

		auto interop = m_compositor.as<abi::ICompositorDesktopInterop>();

		interop->CreateDesktopWindowTarget(m_window, true, reinterpret_cast<abi::IDesktopWindowTarget**>(winrt::put_abi(m_target)));

		m_rootVisual = m_compositor.CreateContainerVisual();
		m_rootVisual.RelativeSizeAdjustment({ 1.0f, 1.0f });
		m_target.Root(m_rootVisual);

		m_caption = m_compositor.CreateContainerVisual();
		m_caption.Size({ XHK_TITLEBAR_CAPTION_WIDTH, static_cast<float>(Helpers::GetCaptionSize(m_window)) });
		m_caption.Offset({ -XHK_TITLEBAR_CAPTION_WIDTH, static_cast<float>(Helpers::GetTopBorderSize(m_window)), 0.0f });
		m_caption.RelativeOffsetAdjustment({ 1.0, 0.0f, 0.0f });
		m_rootVisual.Children().InsertAtTop(m_caption);

		auto captionBg = m_compositor.CreateSpriteVisual();
		captionBg.RelativeSizeAdjustment({ 1.0f, 1.0f });
		auto brush = m_compositor.CreateColorBrush({ 255, 0, 120, 215 });
		captionBg.Brush(brush);

		auto test = m_compositor.CreateRectangleGeometry();
		test.Size({ 10.0f, 10.0f });
		test.Offset({ 0.0f, 0.0f });

		m_caption.Children().InsertAtBottom(captionBg);
		m_caption.Children().InsertAtTop(CreateCaptionButton(m_compositor, test, 1));
		m_caption.Children().InsertAtTop(CreateCaptionButton(m_compositor, test, 2));
		m_caption.Children().InsertAtTop(CreateCaptionButton(m_compositor, test, 3));

		m_caption.IsVisible(m_extend);

		SetPropW(m_window, XHK_TITLEBAR_OBJECT_PROP, this);

		SetWindowSubclass(m_window, XamlWindowSubClassProc, 1, NULL);
	}

	bool LegacyTitleBar::ExtendViewIntoTitleBar() const {
		return m_extend;
	}

	void LegacyTitleBar::ExtendViewIntoTitleBar(bool const& value) {
		m_extend = value;
		m_caption.IsVisible(value);
		SetWindowPos(m_window, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		
		auto strong_this = this->get_strong();
		m_layoutMetricsChanged(strong_this.as<CoreApplicationViewTitleBar>(), nullptr);
	}

	float LegacyTitleBar::Height() const
	{
		return static_cast<float>(Helpers::GetCaptionSize(m_window));
	}

	bool LegacyTitleBar::IsVisible() const
	{
		return m_isVisible;
	}

	float LegacyTitleBar::SystemOverlayLeftInset() const
	{
		return 0;
	}

	float LegacyTitleBar::SystemOverlayRightInset() const
	{
		return m_extend ? XHK_TITLEBAR_CAPTION_WIDTH : 0;
	}

	winrt::event_token LegacyTitleBar::IsVisibleChanged(TypedEventHandler<CoreApplicationViewTitleBar, IInspectable> const& handler)
	{
		return m_isVisibleChanged.add(handler);
	}

	void LegacyTitleBar::IsVisibleChanged(winrt::event_token const& token) noexcept
	{
		m_isVisibleChanged.remove(token);
	}

	winrt::event_token LegacyTitleBar::LayoutMetricsChanged(TypedEventHandler<CoreApplicationViewTitleBar, IInspectable> const& handler)
	{
		return m_layoutMetricsChanged.add(handler);
	}

	void LegacyTitleBar::LayoutMetricsChanged(winrt::event_token const& token) noexcept
	{
		m_layoutMetricsChanged.remove(token);
	}

	LRESULT LegacyTitleBar::XamlWindowSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR idSubclass, DWORD_PTR dwRefData)
	{
		auto _this = reinterpret_cast<LegacyTitleBar*>(GetPropW(hwnd, XHK_TITLEBAR_OBJECT_PROP));

		if (msg == WM_DESTROY)
		{
			RemovePropW(hwnd, XHK_TITLEBAR_OBJECT_PROP);
		}
		else if (_this)
		{
			if (msg == WM_SIZE)
			{
				auto width = static_cast<float>(LOWORD(lParam));
				auto height = static_cast<float>(HIWORD(lParam));

				_this->m_caption.Offset({ -_this->SystemOverlayRightInset(), static_cast<float>(Helpers::GetTopBorderSize(hwnd)), 0 });
				_this->m_caption.Size({ _this->SystemOverlayRightInset(), static_cast<float>(Helpers::GetCaptionSize(hwnd)) });

			}
			else if (msg == WM_NCHITTEST && _this->m_extend)
			{
				auto x = GET_X_LPARAM(lParam);
				auto y = GET_Y_LPARAM(lParam);
				auto ret = DefWindowProcW(hwnd, msg, wParam, lParam);

				if (ret == HTCLIENT)
				{
					RECT rc;
					RECT rcc;
					GetWindowRect(hwnd, &rc);
					GetClientRect(hwnd, &rcc);
					auto dpi = Helpers::GetDpiForWindow(hwnd);
					auto border = GetSystemMetricsForDpi(SM_CXFRAME, dpi) + GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
					auto caption = Helpers::GetTopBorderSize(hwnd) + Helpers::GetCaptionSize(hwnd);

					if (y > rc.top && y < rc.top + caption)
					{
						auto buttonSize = 46;
						auto relative = rc.right - border;
						if (x < relative && x > relative - buttonSize)
						{

							return HTCLOSE;
						}
						else if (x < relative - buttonSize && x > relative - buttonSize * 2)
						{

							return HTMAXBUTTON;
						}
						else  if (x < relative - buttonSize * 2 && x > relative - buttonSize * 3)
						{

							return HTMINBUTTON;
						}
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
					auto border = GetSystemMetricsForDpi(SM_CXFRAME, dpi) + GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
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
		}

		return DefSubclassProc(hwnd, msg, wParam, lParam);
	}

	ContainerVisual LegacyTitleBar::CreateCaptionButton(Compositor const& compositor, CompositionGeometry const& geometry, int index)
	{
		auto container = compositor.CreateContainerVisual();
		container.Size({ 46.0f, 0.0f });
		container.Offset({ -container.Size().x * index , 0.0f, 0.0f });
		container.RelativeSizeAdjustment({ 0.0f, 1.0f });
		container.RelativeOffsetAdjustment({ 1.0, 0.0f, 0.0f });

		auto background = compositor.CreateSpriteVisual();
		background.RelativeSizeAdjustment({ 1.0f, 1.0f });
		auto brush = compositor.CreateColorBrush({ 255, 42, 42, 42 });
		background.Brush(brush);

		container.Children().InsertAtBottom(background);

		auto color = compositor.CreateColorBrush({ 255, 255, 255, 255 });

		auto sprite = compositor.CreateSpriteShape(geometry);
		sprite.StrokeBrush(color);
		sprite.StrokeThickness(1.0f);

		auto shape = compositor.CreateShapeVisual();
		shape.Size({ 10.0f, 10.0f });
		shape.Offset({ 18.0f, 11.0f, 0.0f });
		shape.Shapes().Append(sprite);

		container.Children().InsertAtTop(shape);

		return container;
	}

}
