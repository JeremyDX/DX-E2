#pragma once

#include <cstdint>

class GameTime
{
public:

	static void Initialize(int UpdateHzRate = 60);
	static void Tick();

	static float GetElapsedRunningTimeInSeconds();

	static uint64_t GetTotalTickLoopsExecuted();

	static uint64_t TotalElapsedClockCycles();

	static uint64_t GetAbsoluteFrameTicks();

	static float GetDeltaElapsedTime();
	static float GetFrameTickDelta();
	static float GetFrameTickLimit();

	static float GetCurrentTimeMicros();

	static bool GetFPSDisplayCounterRate(float FpsUpdateFrequencyInSeconds, float& Out_TotalFPS);
};