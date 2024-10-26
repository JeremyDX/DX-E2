#include "BinaryCacheLoader.h"
#include "BinaryReaderWriter.h"
#include <d3d11.h>      
#include <d3dcompiler.h> 
#include <wrl.h>
#include "Engine.h"
#include "Constants.h"

ID3D11VertexShader** VertexShaders = nullptr;
ID3D11InputLayout** InputLayouts = nullptr;
ID3D11PixelShader** PixelShaders = nullptr;

void BinaryCacheLoader::LoadShaders()
{
	bool Failed = false;

	const char* VertexShaderFiles[] = 
	{
		"VertexShader.hlsl",
		"HeightMapVertexShader.hlsl",
	};

	const char* PixelShaderFiles[] = {
		"PixelShader.hlsl",
		"LandscapePixelShader.hlsl",
	};

	constexpr int VERTEX_ARRAY_SIZE = sizeof(VertexShaderFiles) / sizeof(char*);
	constexpr int PIXEL_ARRAY_SIZE = sizeof(PixelShaderFiles) / sizeof(char*);

	char* ShaderStringData = NULL;
	int ShaderFileLength = 0;

	Microsoft::WRL::ComPtr<ID3D10Blob> ShaderBlob;

	VertexShaders = new ID3D11VertexShader*[VERTEX_ARRAY_SIZE];
	InputLayouts = new ID3D11InputLayout*[VERTEX_ARRAY_SIZE];

	PixelShaders = new ID3D11PixelShader*[PIXEL_ARRAY_SIZE];
	
	for (int i = 0; i < VERTEX_ARRAY_SIZE; ++i)
	{
		BinaryReaderWriter::MallocFileDataInBuffer(VertexShaderFiles[i], ShaderStringData, ShaderFileLength);

		if (ShaderStringData == NULL)
		{
			Failed = true;
			continue;
		}

		HRESULT Result = D3DCompile(ShaderStringData, ShaderFileLength, NULL, NULL, NULL, "main", "vs_5_0", NULL, NULL, ShaderBlob.GetAddressOf(), NULL);

		free(ShaderStringData); //we'll eventually get rid of this for now it's temp...

		if (FAILED(Result))
		{
			Failed = true;
			continue;
		}

		Engine::device->CreateVertexShader(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), NULL, &VertexShaders[i]);
		Engine::device->CreateInputLayout(Constants::Layout_Byte32, 3, ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), &InputLayouts[i]);
	}

	for (int i = 0; i < PIXEL_ARRAY_SIZE; ++i)
	{
		BinaryReaderWriter::MallocFileDataInBuffer(PixelShaderFiles[i], ShaderStringData, ShaderFileLength);

		if (ShaderStringData == NULL)
		{
			Failed = true;
			continue;
		}

		HRESULT Result = D3DCompile(ShaderStringData, ShaderFileLength, NULL, NULL, NULL, "main", "ps_5_0", NULL, NULL, ShaderBlob.GetAddressOf(), NULL);

		free(ShaderStringData); //we'll eventually get rid of this for now it's temp...

		if (FAILED(Result))
		{
			Failed = true;
			continue;
		}

		Engine::device->CreatePixelShader(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), NULL, &PixelShaders[i]);
	}

	ShaderBlob.Reset();
}

void BinaryCacheLoader::UseShaders(const int VertexShaderIndex, const int PixelShaderIndex)
{
/*
	ID3D11VertexShader* OldVertexShader;
	ID3D11PixelShader* OldPixelShader;

	Engine::context->VSGetShader(&OldVertexShader, nullptr, nullptr);
	Engine::context->PSGetShader(&OldPixelShader, nullptr, nullptr);
*/
	Engine::context->VSSetShader(VertexShaders[VertexShaderIndex], nullptr, 0);
	Engine::context->IASetInputLayout(InputLayouts[VertexShaderIndex]);

	Engine::context->PSSetShader(PixelShaders[PixelShaderIndex], nullptr, 0);
}
