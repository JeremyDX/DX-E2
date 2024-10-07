#pragma once

#include <DirectXMath.h>

class CameraEngine
{
	public:
		static void ResetPrimaryCameraMatrix(const int FACE_DIRECTION);
		static bool PrimaryCameraUpdatedLookAt();

	private:
		static void BuildPrimaryCameraMatrix();
		static void UpdatePromptView();

	public:
		static DirectX::XMFLOAT4X4 final_result;
};
