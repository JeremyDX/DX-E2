#pragma once

#include <DirectXMath.h>

class CameraEngine
{
	public:
		static void ResetPrimaryCameraMatrix(const float FACE_DIRECTION);
		static bool PrimaryCameraUpdatedLookAt();
		static void GetDebugString(char* Out_Chars, int CharLength);

	private:
		static void BuildPrimaryCameraMatrix();
		static void UpdatePromptView();

	public:
		static DirectX::XMFLOAT4X4 final_result;
};
