#pragma once

#include <DirectXMath.h>
#include <wrl.h>
#include <d3d11.h>
#include "Constants.h"

struct PerFrameConstantBufferStruct
{
	float DeltaTime;
	float ElapsedTime;
	int FrameIndex;
	float Unused;
};

struct PerCameraChangeConstBufferStruct
{
	float ViewMatrix[16];
	float InverseViewMatrix[16];
	Vector3D CameraPosition;
	float Yaw;
	Vector3D CameraForwardVector;
	float Pitch;
	Vector3D CameraRightVector;
	float Roll;
	Vector3D CameraUpVector;
	float UNUSED;
};

struct VertexShaderCache
{

};

struct PixelShaderCache
{

};

class BinaryCacheLoader
{
	public:

		static void LoadShaders();
		static void UseShaders(const int VertexShaderIndex, const int PixelShaderIndex);
	
	public:

		static Microsoft::WRL::ComPtr<ID3D11Buffer> PerFrameConstBuffer;
		static Microsoft::WRL::ComPtr<ID3D11Buffer> PerCameraChangeConstBuffer;
};