#include "pch.h"
#include "XamlTitleBar.h"
#include "XamlTitleBar.g.cpp"
#include "Helpers.h"
#include <windowsx.h>
#include <dwmapi.h>
#include <shellapi.h>

namespace winrt::XamlHostingKit
{
#ifdef TITLEBAR_USE_VISUALS
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
#endif
}

namespace winrt::XamlHostingKit::implementation
{

	XamlTitleBar::XamlTitleBar(HWND const& xamlWindow, HWND const& coreWindow, Compositor const& compositor, CoreDispatcher const& dispatcher)
		: m_xamlWindow(xamlWindow), m_coreWindow(coreWindow), m_dispatcher(dispatcher), m_compositor(compositor)
	{
		m_isVisible = true;

#ifdef TITLEBAR_USE_VISUALS
		namespace abi = ABI::Windows::UI::Composition::Desktop;

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
#else
		UpdateCaptionColors();

		CreateCompositionDevice();
#endif

		SetPropW(m_xamlWindow, XHK_TITLEBAR_OBJECT_PROP, this);
		SetPropW(m_coreWindow, XHK_TITLEBAR_OBJECT_PROP, this);

		SetWindowSubclass(m_xamlWindow, XamlWindowSubClassProc, 1, NULL);
		SetWindowSubclass(m_coreWindow, CoreWindowSubClassProc, 1, NULL);
	}

	bool XamlTitleBar::ExtendViewIntoTitleBar() const {
		return m_extend;
	}

	void XamlTitleBar::ExtendViewIntoTitleBar(bool const& value) {
		m_extend = value;
#ifdef TITLEBAR_USE_VISUALS
		m_caption.IsVisible(value);
#else
		if (value)
			m_rootVisual->AddVisual(m_caption.get(), TRUE, nullptr);
		else
			m_rootVisual->RemoveVisual(m_caption.get());
#endif

		SetWindowPos(m_xamlWindow, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		if (m_layoutMetricsChanged) {
			m_dispatcher.RunAsync(CoreDispatcherPriority::Normal, [=]()
				{
					m_layoutMetricsChanged(*this, nullptr);
				});
		}
	}

	float XamlTitleBar::Height() const
	{
		return static_cast<float>(Helpers::GetCaptionSize(m_xamlWindow));
	}

	bool XamlTitleBar::IsVisible() const
	{
		return m_isVisible;
	}

	float XamlTitleBar::SystemOverlayLeftInset() const
	{
		return 0;
	}

	float XamlTitleBar::SystemOverlayRightInset() const
	{
		return m_extend ? XHK_TITLEBAR_CAPTION_WIDTH : 0;
	}

	winrt::event_token XamlTitleBar::IsVisibleChanged(TypedEventHandler<winrt::XamlHostingKit::XamlTitleBar, IInspectable> const& handler)
	{
		return m_isVisibleChanged.add(handler);
	}

	void XamlTitleBar::IsVisibleChanged(winrt::event_token const& token) noexcept
	{
		m_isVisibleChanged.remove(token);
	}

	winrt::event_token XamlTitleBar::LayoutMetricsChanged(TypedEventHandler<winrt::XamlHostingKit::XamlTitleBar, IInspectable> const& handler)
	{
		return m_layoutMetricsChanged.add(handler);
	}

	void XamlTitleBar::LayoutMetricsChanged(winrt::event_token const& token) noexcept
	{
		m_layoutMetricsChanged.remove(token);
	}

	void XamlTitleBar::UpdateCaptionColors()
	{
		auto dark = Helpers::ShouldAppsUseDarkMode();

#ifdef TITLEBAR_USE_VISUALS
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
#else
		m_captionOtherBackground = ToColorF(dark ? XHK_TITLEBAR_CAPTION_OTHER_BACKGROUND_DARK : XHK_TITLEBAR_CAPTION_OTHER_BACKGROUND_LIGHT);
		m_captionOtherHoverBackground = ToColorF(dark ? XHK_TITLEBAR_CAPTION_OTHER_HOVER_BACKGROUND_DARK : XHK_TITLEBAR_CAPTION_OTHER_HOVER_BACKGROUND_LIGHT);
		m_captionOtherActiveBackground = ToColorF(dark ? XHK_TITLEBAR_CAPTION_OTHER_ACTIVE_BACKGROUND_DARK : XHK_TITLEBAR_CAPTION_OTHER_ACTIVE_BACKGROUND_LIGHT);

		m_captionCloseBackground = ToColorF(dark ? XHK_TITLEBAR_CAPTION_CLOSE_BACKGROUND_DARK : XHK_TITLEBAR_CAPTION_CLOSE_BACKGROUND_LIGHT);
		m_captionCloseHoverBackground = ToColorF(dark ? XHK_TITLEBAR_CAPTION_CLOSE_HOVER_BACKGROUND_DARK : XHK_TITLEBAR_CAPTION_CLOSE_HOVER_BACKGROUND_LIGHT);
		m_captionCloseActiveBackground = ToColorF(dark ? XHK_TITLEBAR_CAPTION_CLOSE_ACTIVE_BACKGROUND_DARK : XHK_TITLEBAR_CAPTION_CLOSE_ACTIVE_BACKGROUND_LIGHT);

		m_captionForeground = ToColorF(dark ? XHK_TITLEBAR_CAPTION_FOREGROUND_DARK : XHK_TITLEBAR_CAPTION_FOREGROUND_LIGHT);
		m_captionHoverForeground = ToColorF(dark ? XHK_TITLEBAR_CAPTION_HOVER_FOREGROUND_DARK : XHK_TITLEBAR_CAPTION_HOVER_FOREGROUND_LIGHT);
		m_captionActiveForeground = ToColorF(dark ? XHK_TITLEBAR_CAPTION_ACTIVE_FOREGROUND_DARK : XHK_TITLEBAR_CAPTION_ACTIVE_FOREGROUND_LIGHT);
		m_captionInactiveForeground = ToColorF(dark ? XHK_TITLEBAR_CAPTION_INACTIVE_FOREGROUND_DARK : XHK_TITLEBAR_CAPTION_INACTIVE_FOREGROUND_LIGHT);
#endif
	}

#ifndef TITLEBAR_USE_VISUALS

	void XamlTitleBar::CreateCompositionDevice()
	{
		winrt::com_ptr<ID3D11Device> device;

		auto d3dDriverTypes = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP };

		HRESULT d3d11HR = NULL;

		for (const auto& driver : d3dDriverTypes) {
			d3d11HR = D3D11CreateDevice(nullptr, driver, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION, device.put(), nullptr, nullptr);
			if (SUCCEEDED(d3d11HR))
				break;
		}

		winrt::check_hresult(d3d11HR);

		winrt::com_ptr<ID2D1Device> d2d1Device;
		auto pDXGIDevice = device.as<IDXGIDevice>();
		winrt::check_hresult(D2D1CreateDevice(pDXGIDevice.get(), nullptr, d2d1Device.put()));

		winrt::com_ptr<IDCompositionSurfaceFactory> surfaceFactory;
		if (m_compositor && (m_dcompDevice = m_compositor.try_as<IDCompositionDesktopDevice>())) [[likely]]
		{
			winrt::check_hresult(m_dcompDevice->CreateSurfaceFactory(d2d1Device.get(), surfaceFactory.put()));
		}
		else
		{
			winrt::check_hresult(DCompositionCreateDevice2(d2d1Device.get(), __uuidof(IDCompositionDesktopDevice), m_dcompDevice.put_void()));
		}

		winrt::com_ptr<IDCompositionTarget> target;
		winrt::check_hresult(m_dcompDevice->CreateTargetForHwnd(m_xamlWindow, TRUE, target.put()));

		winrt::com_ptr<IDCompositionVisual2> rootVisual;
		winrt::check_hresult(m_dcompDevice->CreateVisual(rootVisual.put()));
		winrt::check_hresult(target->SetRoot(rootVisual.get()));

		winrt::com_ptr<IDCompositionVisual2> caption;
		winrt::check_hresult(m_dcompDevice->CreateVisual(caption.put()));

		if (m_extend)
		{
			winrt::check_hresult(rootVisual->AddVisual(caption.get(), TRUE, nullptr));
		}

		m_d3d11Device = device;
		m_d2d1Device = d2d1Device;
		m_surfaceFactory = surfaceFactory;
		m_target = target;
		m_rootVisual = rootVisual;
		m_caption = caption;

		auto scale = Helpers::GetDpiScaleForWindow(m_xamlWindow);

		CreateCaptionSurface(scale);

		DrawCaption(scale, TitleBarCaptionButtonType::NONE, TitleBarCaptionButtonState::NORMAL);

		RECT clientRect{ };
		GetClientRect(m_xamlWindow, &clientRect);

		auto scaledCaptionSize = XHK_TITLEBAR_CAPTION_WIDTH * scale;

		m_caption->SetOffsetX(clientRect.right - scaledCaptionSize);
		m_caption->SetOffsetY(static_cast<float>(Helpers::GetTopBorderSize(m_xamlWindow)));

		m_dcompDevice->Commit();
	}

	void XamlTitleBar::CreateCaptionSurface(float scale)
	{
		if (!m_dcompDevice) [[unlikely]]
			return;
		if (m_surfaceFactory) [[likely]]
		{
			winrt::check_hresult(m_surfaceFactory->CreateSurface(static_cast<UINT>(XHK_TITLEBAR_CAPTION_WIDTH * scale), static_cast<UINT>(Helpers::GetCaptionSize(m_xamlWindow)), DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_PREMULTIPLIED, m_captionSurface.put()));
		}
		else
		{
			winrt::check_hresult(m_dcompDevice->CreateSurface(static_cast<UINT>(XHK_TITLEBAR_CAPTION_WIDTH * scale), static_cast<UINT>(Helpers::GetCaptionSize(m_xamlWindow)), DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_PREMULTIPLIED, m_captionSurface.put()));
		}
		winrt::check_hresult(m_caption->SetContent(m_captionSurface.get()));
	}

	void XamlTitleBar::DrawCaption(float scale, winrt::XamlHostingKit::TitleBarCaptionButtonType const& buttonType, winrt::XamlHostingKit::TitleBarCaptionButtonState const& buttonState)
	{
		if (!m_dcompDevice) [[unlikely]]
			return;
		POINT offset = {};
		winrt::com_ptr<ID2D1DeviceContext> d2d1Context;
		winrt::check_hresult(m_captionSurface->BeginDraw(nullptr, IID_ID2D1DeviceContext, d2d1Context.put_void(), &offset));

		d2d1Context->SetDpi(96.0f, 96.0f);
		d2d1Context->Clear(D2D1::ColorF(0x0078D7, 0.0f));

		auto scaleMatrix = D2D1::Matrix3x2F::Scale(scale, scale);
		auto translationMatrix = D2D1::Matrix3x2F::Translation(static_cast<FLOAT>(offset.x), static_cast<FLOAT>(offset.y));
		auto baseTransform = scaleMatrix * translationMatrix;

		// Close button
		d2d1Context->SetTransform(D2D1::Matrix3x2F::Translation(XHK_TITLEBAR_CAPTION_WIDTH - XHK_TITLEBAR_CAPTION_BUTTON_WIDTH, 0.0f) * baseTransform);
		{
			D2D1::ColorF bgColor = m_captionCloseBackground;
			D2D1::ColorF stColor = m_isActive ? m_captionForeground : m_captionInactiveForeground;

			if (buttonType == winrt::XamlHostingKit::TitleBarCaptionButtonType::CLOSE)
				CaptionButtonColor(buttonType, buttonState, &bgColor, &stColor);

			winrt::com_ptr<ID2D1SolidColorBrush> background;
			winrt::com_ptr<ID2D1SolidColorBrush> stroke;
			winrt::check_hresult(d2d1Context->CreateSolidColorBrush(bgColor, background.put()));
			winrt::check_hresult(d2d1Context->CreateSolidColorBrush(stColor, stroke.put()));

			auto point = D2D1::Point2F(0.0f, 0.0f);
			auto rectangle = D2D1::RectF(point.x, point.y, point.x + XHK_TITLEBAR_CAPTION_BUTTON_WIDTH, point.y + XHK_TITLEBAR_CAPTION_BUTTON_HEIGHT);
			d2d1Context->FillRectangle(rectangle, background.get());

			point = D2D1::Point2F(18.0f, 11.0f);
			rectangle = D2D1::RectF(point.x, point.y, point.x + 10.0f, point.y + 10.0f);
			d2d1Context->PushAxisAlignedClip(rectangle, D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			{
				auto point1 = D2D1::Point2F(18.0f, 11.0f);
				auto point2 = D2D1::Point2F(28.0f, 21.0f);
				d2d1Context->DrawLine(point1, point2, stroke.get());
				point1 = D2D1::Point2F(18.0f, 21.0f);
				point2 = D2D1::Point2F(28.0f, 11.0f);
				d2d1Context->DrawLine(point1, point2, stroke.get());
			}
			d2d1Context->PopAxisAlignedClip();
		}
		// Max button
		d2d1Context->SetTransform(D2D1::Matrix3x2F::Translation(XHK_TITLEBAR_CAPTION_WIDTH - XHK_TITLEBAR_CAPTION_BUTTON_WIDTH * 2.0f, 0.0f) * baseTransform);
		{
			D2D1::ColorF bgColor = m_captionOtherBackground;
			D2D1::ColorF stColor = m_isActive ? m_captionForeground : m_captionInactiveForeground;

			if (buttonType == winrt::XamlHostingKit::TitleBarCaptionButtonType::MAXIMIZE)
				CaptionButtonColor(buttonType, buttonState, &bgColor, &stColor);

			winrt::com_ptr<ID2D1SolidColorBrush> background;
			winrt::com_ptr<ID2D1SolidColorBrush> stroke;
			winrt::check_hresult(d2d1Context->CreateSolidColorBrush(bgColor, background.put()));
			winrt::check_hresult(d2d1Context->CreateSolidColorBrush(stColor, stroke.put()));

			auto point = D2D1::Point2F(0.0f, 0.0f);
			auto rectangle = D2D1::RectF(point.x, point.y, point.x + XHK_TITLEBAR_CAPTION_BUTTON_WIDTH, point.y + XHK_TITLEBAR_CAPTION_BUTTON_HEIGHT);
			d2d1Context->FillRectangle(rectangle, background.get());

			auto isWin11 = Helpers::OSBuild >= 22000u;
			auto radius = isWin11 ? 1.0f : 0.0f;

			if (m_isMaximized)
			{
				point = D2D1::Point2F(18.5f, 13.5f);
				rectangle = D2D1::RectF(point.x, point.y, point.x + 7.0f, point.y + 7.0f);
				d2d1Context->DrawRoundedRectangle(D2D1::RoundedRect(rectangle, radius, radius), stroke.get(), 1.0f);

				/*point = D2D1::Point2F(19.5f, 11.5f);
				rectangle = D2D1::RectF(point.x, point.y, point.x + 8.0f, point.y + 8.0f);
				d2d1Context->DrawRoundedRectangle(D2D1::RoundedRect(rectangle, 2.0f, 2.0f), stroke, 1.0f);*/

				if (isWin11)
				{
					auto point1 = D2D1::Point2F(20.5f, 11.5f);
					auto point2 = D2D1::Point2F(26.0f, 11.5f);
					d2d1Context->DrawLine(point1, point2, stroke.get());

					point1 = D2D1::Point2F(26.0f, 11.5f);
					point2 = D2D1::Point2F(27.5f, 13.0f);
					d2d1Context->DrawLine(point1, point2, stroke.get());

					point1 = D2D1::Point2F(27.5f, 13.0f);
					point2 = D2D1::Point2F(27.5f, 18.5f);
					d2d1Context->DrawLine(point1, point2, stroke.get());
				}
				else
				{
					auto point1 = D2D1::Point2F(20.5f, 13.5f);
					auto point2 = D2D1::Point2F(20.5f, 11.5f);
					d2d1Context->DrawLine(point1, point2, stroke.get());

					point1 = D2D1::Point2F(20.0f, 11.5f);
					point2 = D2D1::Point2F(28.0f, 11.5f);
					d2d1Context->DrawLine(point1, point2, stroke.get());

					point1 = D2D1::Point2F(27.5f, 11.5f);
					point2 = D2D1::Point2F(27.5f, 19.0f);
					d2d1Context->DrawLine(point1, point2, stroke.get());

					point1 = D2D1::Point2F(25.5f, 18.5f);
					point2 = D2D1::Point2F(27.5f, 18.5f);
					d2d1Context->DrawLine(point1, point2, stroke.get());
				}
			}
			else
			{
				point = D2D1::Point2F(18.5f, 11.5f);
				rectangle = D2D1::RectF(point.x, point.y, point.x + 9.0f, point.y + 9.0f);
				d2d1Context->DrawRoundedRectangle(D2D1::RoundedRect(rectangle, radius, radius), stroke.get(), 1.0f);
			}
		}

		// Min button
		d2d1Context->SetTransform(D2D1::Matrix3x2F::Translation(XHK_TITLEBAR_CAPTION_WIDTH - XHK_TITLEBAR_CAPTION_BUTTON_WIDTH * 3.0f, 0.0f) * baseTransform);
		{
			D2D1::ColorF bgColor = m_captionOtherBackground;
			D2D1::ColorF stColor = m_isActive ? m_captionForeground : m_captionInactiveForeground;

			if (buttonType == winrt::XamlHostingKit::TitleBarCaptionButtonType::MINIMIZE)
				CaptionButtonColor(buttonType, buttonState, &bgColor, &stColor);

			winrt::com_ptr<ID2D1SolidColorBrush> background;
			winrt::com_ptr<ID2D1SolidColorBrush> stroke;
			winrt::check_hresult(d2d1Context->CreateSolidColorBrush(bgColor, background.put()));
			winrt::check_hresult(d2d1Context->CreateSolidColorBrush(stColor, stroke.put()));

			auto point = D2D1::Point2F(0.0f, 0.0f);
			auto rectangle = D2D1::RectF(point.x, point.y, point.x + XHK_TITLEBAR_CAPTION_BUTTON_WIDTH, point.y + XHK_TITLEBAR_CAPTION_BUTTON_HEIGHT);
			d2d1Context->FillRectangle(rectangle, background.get());

			auto point1 = D2D1::Point2F(18.0f, 15.5f);
			auto point2 = D2D1::Point2F(28.0f, 15.5f);
			d2d1Context->DrawLine(point1, point2, stroke.get());
		}

		m_captionSurface->EndDraw();
	}

	void XamlTitleBar::CaptionButtonColor(winrt::XamlHostingKit::TitleBarCaptionButtonType const& buttonType, winrt::XamlHostingKit::TitleBarCaptionButtonState const& buttonState, D2D1::ColorF* bgColor, D2D1::ColorF* stColor)
	{
		switch (buttonState)
		{
		case winrt::XamlHostingKit::TitleBarCaptionButtonState::HOVER:
			*bgColor = buttonType == winrt::XamlHostingKit::TitleBarCaptionButtonType::CLOSE ? m_captionCloseHoverBackground : m_captionOtherHoverBackground;
			*stColor = m_captionHoverForeground;
			break;
		case winrt::XamlHostingKit::TitleBarCaptionButtonState::ACTIVE:
			*bgColor = buttonType == winrt::XamlHostingKit::TitleBarCaptionButtonType::CLOSE ? m_captionCloseActiveBackground : m_captionOtherActiveBackground;
			*stColor = m_captionActiveForeground;
			break;
		}
	}

	void XamlTitleBar::CommitComposition()
	{
		if (m_dcompDevice) [[unlikely]]
			m_dcompDevice->Commit();
	}

	void XamlTitleBar::ReleaseResources()
	{
		m_captionSurface = nullptr;
		m_caption = nullptr;
		m_rootVisual = nullptr;
		m_target = nullptr;
		m_dcompDevice = nullptr;
		m_surfaceFactory = nullptr;
		m_d2d1Device = nullptr;
		m_d3d11Device = nullptr;
	}
#endif

	LRESULT XamlTitleBar::XamlWindowSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
	{
		auto _this = reinterpret_cast<XamlTitleBar*>(GetPropW(hwnd, XHK_TITLEBAR_OBJECT_PROP));

		if (msg == WM_DESTROY)
		{
#ifndef TITLEBAR_USE_VISUALS
			_this->ReleaseResources();
#endif
			RemovePropW(hwnd, XHK_TITLEBAR_OBJECT_PROP);
		}
		else if (_this)
		{
			if (msg == WM_SIZE)
			{
				auto scaling = Helpers::GetDpiScaleForWindow(hwnd);
				auto scaledCaptionSize = XHK_TITLEBAR_CAPTION_WIDTH * scaling;

#ifdef TITLEBAR_USE_VISUALS
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
#else
				auto width = LOWORD(lParam);
				auto height = HIWORD(lParam);

				_this->m_isMaximized = wParam == SIZE_MAXIMIZED;

				_this->m_caption->SetOffsetX(width - scaledCaptionSize);
				_this->m_caption->SetOffsetY(static_cast<float>(Helpers::GetTopBorderSize(hwnd)));

				if (!_this->m_surfaceFactory)
				{
					// Not running on XAML's dcomp, manually commit this resize
					_this->CommitComposition();
				}
#endif

			}
			else if (msg == WM_DPICHANGED)
			{
#ifndef TITLEBAR_USE_VISUALS
				auto dpi = static_cast<float>(LOWORD(wParam));
				auto scale = dpi / 96.0f;
				_this->CreateCaptionSurface(scale);
				_this->DrawCaption(scale, TitleBarCaptionButtonType::NONE, TitleBarCaptionButtonState::NORMAL);
				_this->CommitComposition();
#endif
			}
			else if (msg == WM_NCHITTEST && _this->m_extend)
			{
				auto x = GET_X_LPARAM(lParam);
				auto y = GET_Y_LPARAM(lParam);

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
				else if (y < rc.top + border + caption)
					return HTCAPTION;
				else
					DefSubclassProc(hwnd, msg, wParam, lParam);
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
#ifdef TITLEBAR_USE_VISUALS
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
#else
				_this->DrawCaption(Helpers::GetDpiScaleForWindow(hwnd), static_cast<TitleBarCaptionButtonType>(wParam), TitleBarCaptionButtonState::ACTIVE);
				_this->CommitComposition();

#endif
				if (wParam == HTMINBUTTON ||
					wParam == HTMAXBUTTON ||
					wParam == HTCLOSE)
				{
					return 0;
				}
			}
			else if (msg == WM_NCLBUTTONUP)
			{
				switch (wParam)
				{
				case HTCLOSE:
#ifdef TITLEBAR_USE_VISUALS
					_this->m_captionClose.Update(_this->m_captionHoverForeground, _this->m_captionCloseHoverBackground);
#endif
					SendMessageW(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
					break;
				case HTMAXBUTTON:
#ifdef TITLEBAR_USE_VISUALS
					_this->m_captionMaximize.Update(_this->m_captionHoverForeground, _this->m_captionOtherHoverBackground);
#endif
					SendMessageW(hwnd, WM_SYSCOMMAND, IsZoomed(hwnd) ? SC_RESTORE : SC_MAXIMIZE, 0);
					break;
				case HTMINBUTTON:
#ifdef TITLEBAR_USE_VISUALS
					_this->m_captionMinimize.Update(_this->m_captionHoverForeground, _this->m_captionOtherHoverBackground);
#endif
					SendMessageW(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
					break;
				}
#ifndef TITLEBAR_USE_VISUALS
				_this->DrawCaption(Helpers::GetDpiScaleForWindow(hwnd), static_cast<TitleBarCaptionButtonType>(wParam), TitleBarCaptionButtonState::HOVER);
				_this->CommitComposition();
#endif
			}
			else if (msg == WM_NCPOINTERUP)
			{
				if (IS_POINTER_FIRSTBUTTON_WPARAM(wParam))
				{
					switch (HIWORD(wParam))
					{
					case HTCLOSE:
#ifdef TITLEBAR_USE_VISUALS
						_this->m_captionClose.Update(_this->m_captionHoverForeground, _this->m_captionCloseHoverBackground);
#endif
						SendMessageW(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
						break;
					case HTMAXBUTTON:
#ifdef TITLEBAR_USE_VISUALS
						_this->m_captionMaximize.Update(_this->m_captionHoverForeground, _this->m_captionOtherHoverBackground);
#endif
						SendMessageW(hwnd, WM_SYSCOMMAND, IsZoomed(hwnd) ? SC_RESTORE : SC_MAXIMIZE, 0);
						break;
					case HTMINBUTTON:
#ifdef TITLEBAR_USE_VISUALS
						_this->m_captionMinimize.Update(_this->m_captionHoverForeground, _this->m_captionOtherHoverBackground);
#endif
						SendMessageW(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
						break;
					}
				}
#ifndef TITLEBAR_USE_VISUALS
				_this->DrawCaption(Helpers::GetDpiScaleForWindow(hwnd), static_cast<TitleBarCaptionButtonType>(wParam), TitleBarCaptionButtonState::HOVER);
				_this->CommitComposition();
#endif
			}
			else if (msg == WM_NCMOUSEMOVE)
			{
#ifdef TITLEBAR_USE_VISUALS
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
#else
				_this->DrawCaption(Helpers::GetDpiScaleForWindow(hwnd), static_cast<TitleBarCaptionButtonType>(wParam), TitleBarCaptionButtonState::HOVER);
				_this->CommitComposition();
#endif
			}
			else if (msg == WM_NCMOUSELEAVE || msg == WM_MOUSELEAVE)
			{
#ifdef TITLEBAR_USE_VISUALS
				_this->m_captionClose.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionCloseBackground);
				_this->m_captionMaximize.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionOtherBackground);
				_this->m_captionMinimize.Update(_this->m_isActive ? _this->m_captionForeground : _this->m_captionInactiveForeground, _this->m_captionOtherBackground);
#else
				_this->DrawCaption(Helpers::GetDpiScaleForWindow(hwnd), static_cast<TitleBarCaptionButtonType>(wParam), TitleBarCaptionButtonState::HOVER);
				_this->CommitComposition();
#endif
			}
			else if (msg == WM_NCACTIVATE)
			{
#ifdef TITLEBAR_USE_VISUALS
				if ((_this->m_isActive = wParam))
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
#else
				_this->m_isActive = wParam;
				_this->DrawCaption(Helpers::GetDpiScaleForWindow(hwnd), static_cast<TitleBarCaptionButtonType>(wParam), TitleBarCaptionButtonState::NORMAL);
				_this->CommitComposition();
#endif
			}
			else if (msg == WM_SETTINGCHANGE)
			{
				_this->UpdateCaptionColors();
			}
			else if (msg == WM_PAINT)
			{
#ifndef TITLEBAR_USE_VISUALS
				auto hr = _this->m_d3d11Device->GetDeviceRemovedReason();
				if (!SUCCEEDED(hr))
				{
					_this->ReleaseResources();
					_this->CreateCompositionDevice();
				}
#endif
			}
		}

		return DefSubclassProc(hwnd, msg, wParam, lParam);
	}

	LRESULT XamlTitleBar::CoreWindowSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
	{
		auto _this = reinterpret_cast<XamlTitleBar*>(GetPropW(hwnd, XHK_TITLEBAR_OBJECT_PROP));

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

#ifdef TITLEBAR_USE_VISUALS
	winrt::XamlHostingKit::TitleBarCaptionButton XamlTitleBar::CreateCaptionButton(Compositor const& compositor, std::vector<CompositionGeometry> const& geometry, std::vector<CompositionGeometry> const& geometry_ext, int index)
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
#else
	D2D1::ColorF XamlTitleBar::ToColorF(Color const& color)
	{
		return D2D1::ColorF((color.R << 16) | (color.G << 8) | color.B, color.A / 255.0f);
	}
#endif

}
