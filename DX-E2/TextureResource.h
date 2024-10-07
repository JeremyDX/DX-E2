#pragma once

#include <wrl/client.h>
#include <d3d11.h>

class TextureResource
{
	public:
		TextureResource();
		~TextureResource();
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
};