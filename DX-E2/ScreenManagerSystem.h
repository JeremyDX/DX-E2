#pragma once

#include <cstdint>

class ScreenManagerSystem
{

	public:
		static void UpdateScreenParameters(const uint16_t resolution_height, const uint16_t resolution_width);

		static float GetScreenAspectRatio();

		static uint16_t GetScreenWidth();
		static uint16_t GetScreenHeight();
};