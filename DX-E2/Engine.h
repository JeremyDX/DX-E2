#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class Engine
{
	public:
		static void Stop();
		static int StartGameLoop(void* vRawHWNDPtr);

	public:
		static Microsoft::WRL::ComPtr<ID3D11Device> device;
		static Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
};
