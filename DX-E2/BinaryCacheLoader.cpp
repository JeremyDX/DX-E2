#include "BinaryCacheLoader.h"
#include "BinaryReaderWriter.h"
#include <d3d11.h>      
#include <d3dcompiler.h> 
#include <wrl.h>
#include "Engine.h"
#include "Constants.h"

Microsoft::WRL::ComPtr<ID3D11Buffer> BinaryCacheLoader::PerFrameConstBuffer;
Microsoft::WRL::ComPtr<ID3D11Buffer> BinaryCacheLoader::PerCameraChangeConstBuffer;

ID3D11VertexShader** VertexShaders = nullptr;
ID3D11InputLayout** InputLayouts = nullptr;
ID3D11PixelShader** PixelShaders = nullptr;

struct ShaderFileStorage
{
	const char* VertexShaderName;
	const char* PixelShaderName;
	uint8_t ElementSize;
	const D3D11_INPUT_ELEMENT_DESC* LayoutType;
};

void BinaryCacheLoader::LoadShaders()
{
	bool Failed = false;

	ShaderFileStorage ShaderFiles[4];

	ShaderFiles[static_cast<int>(ShaderEnumTypes::MAIN_UI)]					= { "VertexShader.hlsl",			 "PixelShader.hlsl",			 3,	Constants::Layout_Byte32	};
	ShaderFiles[static_cast<int>(ShaderEnumTypes::TERRAIN_HEIGHT_TEST1)]	= { "HeightMapVertexShader.hlsl",	 "HeightMapPixelShader.hlsl",	 1,	Constants::Layout_PackedInt	};
	ShaderFiles[static_cast<int>(ShaderEnumTypes::WATER_TEST1)]				= { "WaterVertexShader.hlsl",		 "WaterPixelShader.hlsl",		 1,	Constants::Layout_PackedInt	};
	ShaderFiles[static_cast<int>(ShaderEnumTypes::TERRAIN_REGION_TEST1)]	= { "LandscapeVertexLodShader.hlsl", "LandscapePixelLodShader.hlsl", 1,	Constants::Layout_PackedInt	};

	constexpr int SHADER_SIZE = sizeof(ShaderFiles) / sizeof(ShaderFiles[0]);

	char* ShaderStringData = NULL;
	int ShaderFileLength = 0;

	Microsoft::WRL::ComPtr<ID3D10Blob> ShaderBlob;

	VertexShaders = new ID3D11VertexShader*[SHADER_SIZE];
	InputLayouts = new ID3D11InputLayout*[SHADER_SIZE];
	PixelShaders = new ID3D11PixelShader*[SHADER_SIZE];
	
	for (int i = 0; i < SHADER_SIZE; ++i)
	{
		BinaryReaderWriter::MallocFileDataInBuffer(ShaderFiles[i].VertexShaderName, ShaderStringData, ShaderFileLength);

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

		OutputDebugStringW(L"Created Shader");

		Engine::device->CreateVertexShader(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), NULL, &VertexShaders[i]);
		Engine::device->CreateInputLayout(ShaderFiles[i].LayoutType, ShaderFiles[i].ElementSize, ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), &InputLayouts[i]);
		//Engine::device->CreateInputLayout(Constants::Layout_PackedInt, 3, ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), &InputLayouts[i]);
	}

	for (int i = 0; i < SHADER_SIZE; ++i)
	{
		BinaryReaderWriter::MallocFileDataInBuffer(ShaderFiles[i].PixelShaderName, ShaderStringData, ShaderFileLength);

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
