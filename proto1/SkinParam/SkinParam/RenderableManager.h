#pragma once

#include <D3D11.h>

namespace Skin {

class RenderableManager /* interface */ {
public:
	virtual void onCreateDevice(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, IDXGISwapChain* pSwapChain) = 0;
	virtual void onResizedSwapChain(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) = 0;
	virtual void onReleasingSwapChain() = 0;
	virtual void onDestroyDevice() = 0;
};

} // namespace Skin
