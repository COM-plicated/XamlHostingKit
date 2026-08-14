#include "pch.h"
#include "TitleBar.h"
#include "TitleBar.g.cpp"
#include "Helpers.h"
#include <windowsx.h>
#include <dwmapi.h>
#include <shellapi.h>

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

		auto captionBg = m_compositor.CreateSpriteVisual();
		captionBg.RelativeSizeAdjustment({ 1.0f, 1.0f });
		auto brush = m_compositor.CreateColorBrush({ 255, 0, 120, 215 });
		captionBg.Brush(brush);

		auto test = m_compositor.CreateRectangleGeometry();
		test.Size({ 10.0f, 10.0f });
		test.Offset({ 0.0f, 0.0f });

		m_caption.Children().InsertAtBottom(captionBg);
		m_caption.Children().InsertAtTop(m_captionClose = CreateCaptionButton(m_compositor, test, 1));
		m_caption.Children().InsertAtTop(m_captionMaximize = CreateCaptionButton(m_compositor, test, 2));
		m_caption.Children().InsertAtTop(m_captionMinimize = CreateCaptionButton(m_compositor, test, 3));

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
				_this->m_captionClose.Size({ scaledButtonSize, 0.0f });
				_this->m_captionClose.Offset({ -scaledButtonSize , 0.0f, 0.0f });

				_this->m_captionMaximize.Size({ scaledButtonSize, 0.0f });
				_this->m_captionMaximize.Offset({ -scaledButtonSize * 2 , 0.0f, 0.0f });

				_this->m_captionMinimize.Size({ scaledButtonSize, 0.0f });
				_this->m_captionMinimize.Offset({ -scaledButtonSize * 3 , 0.0f, 0.0f });

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
					auto border = Helpers::GetSystemMetricsForDpi(SM_CXFRAME, dpi) + Helpers::GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
					auto caption = Helpers::GetTopBorderSize(hwnd) + Helpers::GetCaptionSize(hwnd);

					if (y > rc.top && y < rc.top + caption)
					{
						auto buttonSize = 46.0f * Helpers::GetDpiScaleForWindow(hwnd);
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

	ContainerVisual TitleBar::CreateCaptionButton(Compositor const& compositor, CompositionGeometry const& geometry, int index)
	{
		auto container = compositor.CreateContainerVisual();
		container.Size({ XHK_TITLEBAR_CAPTION_BUTTON_WIDTH, 0.0f });
		container.Offset({ -XHK_TITLEBAR_CAPTION_BUTTON_WIDTH * index , 0.0f, 0.0f });
		container.RelativeSizeAdjustment({ 0.0f, 1.0f });
		container.RelativeOffsetAdjustment({ 1.0f, 0.0f, 0.0f });

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
		//shape.Offset({ 18.0f, 11.0f, 0.0f });
		shape.Shapes().Append(sprite);
		shape.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0.0f });
		shape.AnchorPoint({ 0.5f, 0.5f });

		container.Children().InsertAtTop(shape);

		return container;
	}

}
