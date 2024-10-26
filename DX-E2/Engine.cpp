#include "Engine.h"
#include "Constants.h"
#include "CameraEngine.h"
#include "XGameInput.h"
#include "XModelMesh.h"
#include "ContentLoader.h"
#include "GameTime.h"
#include "Animation.h"
#include "ScreenManagerSystem.h"
#include <d3dcompiler.h>
#include "BinaryReaderWriter.h"
#include "BinaryCacheLoader.h"
#include <cstdio>
#include <dxgi1_4.h>

using namespace Microsoft::WRL;

static int ENGINE_ERROR_CODE = 0x0;

static const UINT VSYNC_ON = 0;
static const float BackgroundColor[4] = { 0.05f, 0.0f, 0.0f, 1.0f };

ComPtr<ID3D11DeviceContext> Engine::context;

ComPtr<ID3D11Device> Engine::device;

ComPtr<IDXGISwapChain> swapchain;

ComPtr<ID3D11RenderTargetView> rendertarget;

ComPtr<ID3D11VertexShader> vertexshader;
ComPtr<ID3D11PixelShader> pixelshader;
ComPtr<ID3D11InputLayout> inputlayout;

ComPtr<ID3D11Buffer> d3d_const_buffer;
ComPtr<ID3D11Buffer> d2d_const_buffer;

ComPtr<ID3D11Buffer> vertexbuffer;

ComPtr<ID3D11DepthStencilView> zbuffer;

ComPtr<ID3D11RasterizerState> rasterizerstate;

ComPtr<ID3D11BlendState> blendstate;

ComPtr<ID3D11DepthStencilState> depthonstate;
ComPtr<ID3D11DepthStencilState> depthoffstate;

ComPtr<ID3D11SamplerState> sampler;

void CreatePipeline()
{
	BinaryCacheLoader::LoadShaders();

	// define and set the constant buffer
	D3D11_BUFFER_DESC bd = { 0 };

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = 64U;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	constexpr DirectX::XMFLOAT4X4 IDENTITY =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	D3D11_SUBRESOURCE_DATA view_srd = { &IDENTITY, 0, 0 };

	Engine::device->CreateBuffer(&bd, nullptr, &d3d_const_buffer);
	Engine::device->CreateBuffer(&bd, &view_srd, &d2d_const_buffer);

	Engine::context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void PostLevelLoading()
{
	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; // Point filtering (no interpolation)
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;  // Clamp addressing mode for U coordinate
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;  // Clamp addressing mode for V coordinate
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;  // Clamp addressing mode for W coordinate

	// Create the sampler state
	ID3D11SamplerState* SamplerState = nullptr;

	HRESULT HR8 = Engine::device->CreateSamplerState(&SamplerDesc, &SamplerState);

	ID3D11ShaderResourceView** Resource3 = ContentLoader::GetTextureAddress(3);

	Engine::context->VSSetShaderResources(0, 1, Resource3); // Bind height map to slot t0
	Engine::context->VSSetSamplers(0, 1, &SamplerState);

	Engine::context->UpdateSubresource(d3d_const_buffer.Get(), 0, 0, &CameraEngine::final_result, 0, 0);

	Engine::context->VSSetConstantBuffers(0, 1, d3d_const_buffer.GetAddressOf());
}


void InitializeDirectXProperties(const HWND& hWnd)
{
	// Set up the swap chain description
	DXGI_SWAP_CHAIN_DESC scd = {};
	scd.BufferCount = 2;
	scd.BufferDesc.Width = ScreenManagerSystem::GetScreenWidth();
	scd.BufferDesc.Height = ScreenManagerSystem::GetScreenHeight();
	scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	scd.SampleDesc.Count = 1;
	scd.BufferDesc.RefreshRate.Numerator = 360;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 1;
	scd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1, // Direct3D 11.1
		D3D_FEATURE_LEVEL_11_0, // Direct3D 11.0
		D3D_FEATURE_LEVEL_10_1, // Direct3D 10.1
		D3D_FEATURE_LEVEL_10_0, // Direct3D 10.0
		D3D_FEATURE_LEVEL_9_3,  // Direct3D 9.3
		D3D_FEATURE_LEVEL_9_2,  // Direct3D 9.2
		D3D_FEATURE_LEVEL_9_1   // Direct3D 9.1
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	HRESULT Result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, featureLevels, numFeatureLevels,
		D3D11_SDK_VERSION, &scd, swapchain.GetAddressOf(), Engine::device.GetAddressOf(), NULL, &Engine::context);

	// get a pointer directly to the back buffer
	Microsoft::WRL::ComPtr<ID3D11Texture2D> BackBuffer;
	swapchain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));

	// create a render target pointing to the back buffer
	Engine::device->CreateRenderTargetView(BackBuffer.Get(), nullptr, &rendertarget);

	D3D11_TEXTURE2D_DESC texd = { 0 };

	texd.Width = ScreenManagerSystem::GetScreenWidth();
	texd.Height = ScreenManagerSystem::GetScreenHeight();
	texd.ArraySize = 1;
	texd.MipLevels = 1;
	texd.SampleDesc.Count = 1;
	texd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ComPtr<ID3D11Texture2D> zbuffertexture;

	Engine::device->CreateTexture2D(&texd, nullptr, &zbuffertexture);

	Engine::device->CreateDepthStencilView(zbuffertexture.Get(), NULL, &zbuffer);

	D3D11_VIEWPORT viewport = { 0 };

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = ScreenManagerSystem::GetScreenWidth();
	viewport.Height = ScreenManagerSystem::GetScreenHeight();
	viewport.MinDepth = 0.0F;
	viewport.MaxDepth = 1.0F;

	Engine::context->RSSetViewports(1, &viewport);

	D3D11_RASTERIZER_DESC rd;
	rd.CullMode = D3D11_CULL_NONE;
	rd.FillMode = D3D11_FILL_SOLID;
	rd.FrontCounterClockwise = TRUE;
	rd.DepthClipEnable = TRUE;
	rd.ScissorEnable = FALSE;
	rd.AntialiasedLineEnable = FALSE;
	rd.MultisampleEnable = FALSE;
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0.0f;
	rd.SlopeScaledDepthBias = 0.0f;

	Engine::device->CreateRasterizerState(&rd, &rasterizerstate);

	Engine::context->RSSetState(rasterizerstate.Get());

	D3D11_BLEND_DESC bd;

	//Allow Blending :)
	bd.RenderTarget[0].BlendEnable = TRUE;

	//Set the Source RGB * Source Alpha.
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;

	//Set the Destination.RGB * (1 - Source.Alpha).
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	//When Comparing the BlendSrc and BlendDest Add Them Together.
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	//When Comparing the SrcAlpha and DestAlpha Find which is greater. (Should always be destination based on render order).
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;

	//No Operations just return the SrcAlpha.
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;

	//No Operations just return the DestAlpha.
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_SRC_ALPHA;

	//UnsureUnused. 
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//Seems to be a situation I'm not utilziing at the moment.
	bd.IndependentBlendEnable = FALSE;

	//Per MSDN - Set To False if ONLY RenderTarget[0] is USED.
	bd.AlphaToCoverageEnable = FALSE;

	Engine::device->CreateBlendState(&bd, &blendstate);

	D3D11_DEPTH_STENCIL_DESC dsd = { 0 };

	dsd.DepthEnable = true;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	Engine::device->CreateDepthStencilState(&dsd, depthonstate.GetAddressOf());

	dsd.DepthEnable = false;

	Engine::device->CreateDepthStencilState(&dsd, depthoffstate.GetAddressOf());

	D3D11_SAMPLER_DESC samplerDesc;

	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	Engine::device->CreateSamplerState(&samplerDesc, sampler.GetAddressOf());

	CreatePipeline();

	CameraEngine::ResetPrimaryCameraMatrix(90);

	ContentLoader::AllocateVertexBuffers();
	ContentLoader::LoadContentStage(0); //First Content Batch.
	PostLevelLoading();
	ContentLoader::PresentWindow(0);    //Show Us The First Screen!!

	Animation::LoadAnimations();
}

void Update()
{
	//Handle Updates to Window if we have one open.
	if (ContentLoader::m_index >= 0)
	{
		ContentWindow cw = ContentLoader::GetCurrentWindow();
		cw.update();
	}

	if (ContentLoader::s_index >= 0)
	{
		ContentOverlay co = ContentLoader::GetCurrentOverlay();
		co.update();
	}

	//Update 3D Camera World Space Context.
	//Process Incoming Camera Change Data.
	//Automatic False Return if NO 3D IS IN USE!
	if (ContentLoader::ALLOW_3D_PROCESSING && CameraEngine::PrimaryCameraUpdatedLookAt())
	{
		Engine::context->UpdateSubresource(d3d_const_buffer.Get(), 0, 0, &CameraEngine::final_result, 0, 0);
		Engine::context->VSSetConstantBuffers(0, 1, d3d_const_buffer.GetAddressOf());
	}

	//Submit Fresh Buffer To GPU.
	ContentLoader::SendUpdatedBufferToGpu();
}

UINT stride = sizeof(Vertex32Byte);
UINT offset = 0;

void Render()
{

	//Set the RenderTarget to the Swapped Buffer So We Can Draw To It!
	Engine::context->ClearDepthStencilView(zbuffer.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	Engine::context->OMSetRenderTargets(1, rendertarget.GetAddressOf(), zbuffer.Get());
	Engine::context->ClearRenderTargetView(rendertarget.Get(), BackgroundColor);

	if (ContentLoader::ALLOW_3D_PROCESSING)
	{
		Engine::context->VSSetShaderResources(0, 1, ContentLoader::GetTextureAddress(4));

		BinaryCacheLoader::UseShaders(1, 1);	
	
		Engine::context->PSSetShaderResources(0, 1, ContentLoader::GetTextureAddress(3));
		Engine::context->PSSetSamplers(0, 1, sampler.GetAddressOf());

		Engine::context->OMSetDepthStencilState(depthonstate.Get(), 0);

		Engine::context->IASetVertexBuffers(0, 1, ContentLoader::static_mesh_buffer.GetAddressOf(), &stride, &offset);
		Engine::context->Draw(ContentLoader::static_mesh_buffer_size, 0);
	}

	//Draw 2D.
	if (ContentLoader::m_index >= 0)
	{
		BinaryCacheLoader::UseShaders(0, 0);

		Engine::context->OMSetDepthStencilState(depthoffstate.Get(), 0);
		Engine::context->PSSetSamplers(0, 1, sampler.GetAddressOf());

		float BlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		Engine::context->OMSetBlendState(blendstate.Get(), BlendFactor, 0xFFFFFFFF);

		ContentWindow cw = ContentLoader::GetCurrentWindow();

		Engine::context->IASetVertexBuffers(0, 1, ContentLoader::static_interfaces_buffer.GetAddressOf(), &stride, &offset);

		if (cw.background_shader_id >= 0)
		{
			//CODE SECITON #1
			Engine::context->PSSetShaderResources(0, 1, ContentLoader::GetTextureAddress(cw.background_shader_id));
			Engine::context->Draw(6, 0);
		}
		for (int i = 0; i < cw.state_changes; ++i)
		{
			//CODE SECITON #2
			Engine::context->PSSetShaderResources(0, 1,
				ContentLoader::GetTextureAddress(cw.state_change_alias[i])
			);
			Engine::context->Draw(cw.state_vertex_sizes[i], cw.state_vertex_offsets[i]);
		}
	}

	if (ContentLoader::s_index >= 0)
	{
		BinaryCacheLoader::UseShaders(0, 0);

		Engine::context->OMSetDepthStencilState(depthoffstate.Get(), 0);

		float BlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		Engine::context->OMSetBlendState(blendstate.Get(), BlendFactor, 0xFFFFFFFF);

		ContentOverlay co = ContentLoader::GetCurrentOverlay();

		Engine::context->IASetVertexBuffers(0, 1, ContentLoader::static_overlay_buffer.GetAddressOf(), &stride, &offset);

		Engine::context->PSSetShaderResources(0, 1, ContentLoader::GetTextureAddress(co.texture_index[0]));
		Engine::context->Draw(co.offsets[0], 0);

		for (int i = 1; i < co.total_textures; ++i)
		{
			Engine::context->PSSetShaderResources(0, 1, ContentLoader::GetTextureAddress(co.texture_index[i]));
			Engine::context->Draw(co.offsets[i], co.offsets[i - 1]);
		}
	}

	swapchain->Present(VSYNC_ON, 0);
}

bool KEEP_LOOPING = true;

double GetGPUMemoryUsageInMB()
{
	ComPtr<IDXGIFactory4> dxgiFactory;
	ComPtr<IDXGIAdapter3> dxgiAdapter;
	DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo = {};

	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))))
	{
		OutputDebugStringW(L"Failed to create DXGIFactory.\n");
		return -1.0f;
	}

	if (FAILED(dxgiFactory->EnumAdapters1(0, reinterpret_cast<IDXGIAdapter1**>(dxgiAdapter.GetAddressOf()))))
	{
		OutputDebugStringW(L"Failed to retrieve DXGI Adapter.\n");
		return -1.0f;
	}

	if (FAILED(dxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo)))
	{
		OutputDebugStringW(L"Failed to query video memory info.\n");
		return -1.0f;
	}

	return static_cast<double>(memoryInfo.CurrentUsage) / 1048576.0;
}

//Force Kill Program.
void Engine::Stop()
{
	KEEP_LOOPING = false;
}

void Engine::Reset()
{
	IDXGIDevice* dxgiDevice = nullptr;
	IDXGIAdapter* dxgiAdapter = nullptr;

	// Get the DXGI device from the D3D11 device
	if (SUCCEEDED(device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice))) {
		// Get the adapter
		if (SUCCEEDED(dxgiDevice->GetAdapter(&dxgiAdapter))) {
			DXGI_ADAPTER_DESC desc;
			dxgiAdapter->GetDesc(&desc);

			// Output memory information
			WCHAR buffer[256];
			swprintf_s(buffer, sizeof(buffer) / sizeof(WCHAR),
				L"Adapter Description: %s\n"
				L"Dedicated Video Memory: %llu MB\n"
				L"Dedicated System Memory: %llu MB\n"
				L"Shared System Memory: %llu MB\n",
				desc.Description,
				desc.DedicatedVideoMemory / (1024 * 1024),
				desc.DedicatedSystemMemory / (1024 * 1024),
				desc.SharedSystemMemory / (1024 * 1024));
			OutputDebugStringW(buffer);

			// Clean up
			dxgiAdapter->Release();
		}
		dxgiDevice->Release();
	}
}

void ClearResources()
{
	ComPtr<ID3D11Debug> debugInterface;
	if (SUCCEEDED(Engine::device.As(&debugInterface))) {
		debugInterface->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}
}

//Only To Be Called Once. Secondary Call Will Kill Execution and End Program.
int Engine::StartGameLoop(void* vRawHWNDPtr)
{
	const HWND hWnd = static_cast<HWND>(vRawHWNDPtr);

	InitializeDirectXProperties(hWnd);

	MSG Msg;

	GameTime::Initialize(); //It's SAFE! to PresentWindow before we begin ticking. It'll initalize as Frame Index 0.
	XGameInput::InitializeDefaultConfigurations();

	while (KEEP_LOOPING)
	{
		while (PeekMessage(&Msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);

			if (Msg.message == WM_QUIT)
			{
				ClearResources();
				return 0;
			}
		}

		XGameInput::LoadAndProcessXboxInputChanges();

		Update();
		Render();

		XGameInput::GameInputPostProcessing();

		GameTime::Tick();

		float TotalFPS;
		if (GameTime::GetFPSDisplayCounterRate(1.0f, TotalFPS))
		{
			char Message[464];
			char CameraDebugString[460];
			CameraEngine::GetDebugString(CameraDebugString, sizeof(CameraDebugString));
			sprintf_s(Message, "FPS: %f (%.6f MS), ABS Ticks: %llu, DebugString: %s, MouseDebug: %llu, GPU Mem: %.3f MB", TotalFPS, 1000.0f / TotalFPS, GameTime::GetAbsoluteFrameTicks(), CameraDebugString, XGameInput::MouseCalls, GetGPUMemoryUsageInMB());
			SetWindowTextA(hWnd, Message);
		}
	}
	ClearResources();

	return 0;
}