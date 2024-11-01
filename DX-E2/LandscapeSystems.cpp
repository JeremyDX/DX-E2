#include "LandscapeSystems.h"
#include "ContentLoader.h"
#include "Engine.h"

extern "C" float ClampASM(float x);

uint16_t* TerrainHeightData = NULL;

float LandscapeSystems::GetCurrentHeightAtLocation(float x, float z)
{
    x = ClampASM(x);
    z = ClampASM(z);

    //The grid in question.
    const int GridX = (static_cast<int>(x) & 1023);
    const int GridZ = 1023 - (static_cast<int>(z) & 1023);

    const int GridX2 = (static_cast<int>(x + 1) & 1023);
    const int GridZ2 = 1023 - (static_cast<int>(z + 1) & 1023);

    //The local position we're within the Grid.
    const float LocalX = x - GridX;
    const float LocalZ = z - (1023 - GridZ);

    if (TerrainHeightData)
    {
        int Grid00 = TerrainHeightData[GridZ * 1024 + GridX];
        int Grid10 = TerrainHeightData[GridZ2 * 1024 + GridX];
        int Grid01 = TerrainHeightData[GridZ * 1024 + GridX2];
        int Grid11 = TerrainHeightData[GridZ2 * 1024 + GridX2];

        float z0  = (Grid00 * (1 - LocalZ)) + (Grid10 * LocalZ);
        float z1 = (Grid01 * (1 - LocalZ)) + (Grid11 * LocalZ);

        float z2 = z0 * (1 - LocalX);
        float z3 = z1 * LocalX;

        float Height = z2 + z3;

        return static_cast<float>((Height / 65535.0) * 200.2);
    }

    return 0;
}

void LandscapeSystems::LoadHeightMapData()
{
	ID3D11ShaderResourceView* ShaderResourceView = *ContentLoader::GetTextureAddress(0);

	ID3D11Resource* ResourceObject = NULL;
	ShaderResourceView->GetResource(&ResourceObject);

	ID3D11Texture2D* ShaderTexture;

	if (FAILED(ResourceObject->QueryInterface(&ShaderTexture)))
	{	
		OutputDebugStringW(L"\nFailed to Load Landscape Height Map - QueryInterface");
		return;
	}

    D3D11_TEXTURE2D_DESC ShaderDescription;
    ShaderTexture->GetDesc(&ShaderDescription);

    D3D11_TEXTURE2D_DESC ReadableDescription = ShaderDescription;
    ReadableDescription.ArraySize = 1;
    ReadableDescription.SampleDesc.Count = 1;
    ReadableDescription.SampleDesc.Quality = 0;
    ReadableDescription.Usage = D3D11_USAGE_STAGING;
    ReadableDescription.BindFlags = 0;
    ReadableDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ReadableDescription.MiscFlags = 0;

    ID3D11Texture2D* ReadableTexture = NULL;
    HRESULT hr2 = Engine::device->CreateTexture2D(&ReadableDescription, NULL, &ReadableTexture);
    if (FAILED(hr2))
    {
        OutputDebugStringW(L"\nFailed to Load Landscape Height Map - CreateTexture2D");
        return;
    }

    Engine::context->CopySubresourceRegion(ReadableTexture, 0, 0, 0, 0, ShaderTexture, 0, nullptr);
	
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    HRESULT hr = Engine::context->Map(ReadableTexture, 0, D3D11_MAP_READ, 0, &MappedResource);

    if (SUCCEEDED(hr)) 
    {
       int16_t* PixelPtr = reinterpret_cast<int16_t*>(MappedResource.pData);

        for (int i = 0; i < 1024 * 1024; ++i)
        {
            if (reinterpret_cast<uint16_t*>(MappedResource.pData)[i] != 0)
            {
                int junk2 = 0;
            }
        }

        TerrainHeightData = new uint16_t[ReadableDescription.Width * ReadableDescription.Height];
        memcpy(TerrainHeightData, MappedResource.pData, ReadableDescription.Width * ReadableDescription.Height * sizeof(int16_t));
        Engine::context->Unmap(ReadableTexture, 0);
    }

    /*
    for (int i = 0; i < 1024*1024; ++i)
    {
        if (TerrainHeightData[i] != 0)
        {
            int junk2 = 0;
        }
    }*/

    //I know these won't get released if we fail at all.. The whole framework design needs some thought.
    //Ideally with "resource creation" i want to queue up the resources to be removed on Function Exit. 
    //I'm not personally a fan of the overhead  this would generate but it would overall be minimal for less code efforts. 
    //So this way failed executions are a bit automatic, but also the whole clean up process for DX11 is automatic.
    //Any CPU based pointers i'll retain as raw non automatic as I don't need it to be automatic. 
    ResourceObject->Release();
    ReadableTexture->Release();
    ShaderTexture->Release();
}
