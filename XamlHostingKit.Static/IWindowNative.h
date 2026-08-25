#pragma once

namespace
{
	DECLARE_INTERFACE_IID_(IWindowNative, IUnknown, "EECDBF0E-BAE9-4CB6-A68E-9598E1CB57BB")
	{
		BEGIN_INTERFACE

			STDMETHOD(get_WindowHandle)(THIS_ OUT HWND * hwnd) PURE;

		END_INTERFACE
	};
}