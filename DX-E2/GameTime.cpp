#include "GameTime.h"

#include <Windows.h>

LARGE_INTEGER StartingTimeStamp;
LARGE_INTEGER PreviousTimeStamp;
LARGE_INTEGER CurrentTimeStamp;

uint64_t TotalRunningCyclesElapsed;				//Initialize Sets to 0 ---------------- Total Clock Cycles Elapsed In the Running Application.
uint64_t LastTickDeltaCycles;					//Initialize Sets to 0 ---------------- Total Clock Cycles Elapsed Since the last Frame Execution.
uint64_t CyclesPerSecond;						//Initialize Sets to QPC Frequency ---- How many Clock Cycles make up a Full Real Second.
uint64_t TotalTickLoopsExecuted;				//Initialize Sets to 0 ---------------- How many times the Tick() function was Executed as a means of tracking things. 
uint64_t DebugLastSavedTickLoops;
uint64_t DebugLastSavedRunningCycles;

float SecondsPerCounterCycle;					//Initialize Sets to 1.0 / Frequency -- How much Real Time makes up a singular clock cycle.
float ClockCyclesPerTargetUpdateHzRate;				//Initialize Sets this to (Frequency / Target Hz Rate). This is how many clock cycles will equal a Target Frame Rate Singular Frame.
float CyclesPerMicrosecond;						//Initialize Sets this to (Frequency / 1 MILLION). This is how many Clock Cycles will create a full MicroSecond. We use float as this is more precise then seconds.

float CurrentTargetFrameTimeInSeconds;
float ElapsedTimeSinceLastHzUpdate;			//Initialize Sets to 0 ---------------- Is the Elapsed Clock Cycles and will be Reset to 0 as a cleanup stage if exceeds Target Frame Rate.

bool HzRateUpdatesBlocked = false;

void GameTime::Initialize(int TargetUpdateHzRate)
{
	LARGE_INTEGER FrequencyCount;

	//This will set the rate at which QPC operates at. Typically will be 10MHz but may bug out 10Khz.
	QueryPerformanceFrequency(&FrequencyCount);
	CyclesPerSecond = FrequencyCount.QuadPart;
	SecondsPerCounterCycle = 1.0f / static_cast<float>(CyclesPerSecond);
	ClockCyclesPerTargetUpdateHzRate = CyclesPerSecond / static_cast<float>(TargetUpdateHzRate); //166666.666667 @ 60hz
	CyclesPerMicrosecond = static_cast<float>(CyclesPerSecond) / 1000000.0f; //In most cases this should be 10.0f

	CurrentTargetFrameTimeInSeconds = 1.0f / TargetUpdateHzRate;

	//Gets the Current TimeStamp that we excute our ticking system.
	QueryPerformanceCounter(&StartingTimeStamp);
	QueryPerformanceCounter(&CurrentTimeStamp);

	TotalRunningCyclesElapsed = 0;
	LastTickDeltaCycles = 0;
	TotalTickLoopsExecuted = 0;

	DebugLastSavedTickLoops = 0;
	DebugLastSavedRunningCycles = 0;

	ElapsedTimeSinceLastHzUpdate = 0;
}

void GameTime::Tick()
{
	PreviousTimeStamp = CurrentTimeStamp;

	QueryPerformanceCounter(&CurrentTimeStamp);

	LastTickDeltaCycles = CurrentTimeStamp.QuadPart - PreviousTimeStamp.QuadPart;
	TotalRunningCyclesElapsed = CurrentTimeStamp.QuadPart - StartingTimeStamp.QuadPart;

	ElapsedTimeSinceLastHzUpdate += LastTickDeltaCycles;

	HzRateUpdatesBlocked = true;
	if (ElapsedTimeSinceLastHzUpdate > ClockCyclesPerTargetUpdateHzRate)
	{
		HzRateUpdatesBlocked = false;
		ElapsedTimeSinceLastHzUpdate -= ClockCyclesPerTargetUpdateHzRate;
	}

	++TotalTickLoopsExecuted;
}

bool GameTime::IsHzBasedUpdateBlocked()
{
	return HzRateUpdatesBlocked;
}

/*
* This is the total elapsed frames for the Update systems i.e. if 10 seconds went by at 60hz we'd be at 600.
* This is to show us where in Frame based operations we exist currently in event of stalls or speed ups so that stuff operates on a set operation.
*/
uint64_t GameTime::GetAbsoluteFrameTicks()
{
	return static_cast<uint64_t>(TotalRunningCyclesElapsed / ClockCyclesPerTargetUpdateHzRate);
}

float GameTime::GetFrameTickLimit()
{
	return CurrentTargetFrameTimeInSeconds;
}

uint64_t GameTime::GetTotalTickLoopsExecuted()
{
	return TotalTickLoopsExecuted;
}

uint64_t GameTime::TotalElapsedClockCycles()
{
	return TotalRunningCyclesElapsed;
}

float GameTime::GetDeltaElapsedTime()
{
	return LastTickDeltaCycles * SecondsPerCounterCycle;
}

float GameTime::GetElapsedRunningTimeInSeconds()
{
	return TotalRunningCyclesElapsed * SecondsPerCounterCycle;
}

/*
* This will return Time Stamps in the form of total microseconds passed.
* I.e. 5 seconds would be 5,000,000 since we operaate at 10mhz we return float so we can get the 0.1 decimals.
*/
float GameTime::GetCurrentTimeMicros()
{
	LARGE_INTEGER TempTimeStamp;
	QueryPerformanceCounter(&TempTimeStamp);
	return static_cast<float>(TempTimeStamp.QuadPart) / CyclesPerMicrosecond;
}

/*
* Sets the OUT parameter to the total game ticks that executed over a second of execution updated at a provided UpdateFrequency
*/
bool GameTime::GetFPSDisplayCounterRate(float FpsUpdateFrequencyInSeconds, float& Out_TotalFPS)
{
	const uint64_t DifferenceOfRunningCycles = TotalRunningCyclesElapsed - DebugLastSavedRunningCycles;

	if (DifferenceOfRunningCycles < (CyclesPerSecond * FpsUpdateFrequencyInSeconds))
	{
		return false;
	}

	const uint64_t DifferenceOfTickLoopsExecuted = GameTime::GetTotalTickLoopsExecuted() - DebugLastSavedTickLoops;
	const float TotalSecondsBetweenFpsUpdates = static_cast<float>(DifferenceOfRunningCycles) / static_cast<float>(CyclesPerSecond);

	Out_TotalFPS = DifferenceOfTickLoopsExecuted / TotalSecondsBetweenFpsUpdates;

	DebugLastSavedTickLoops = GameTime::GetTotalTickLoopsExecuted();
	DebugLastSavedRunningCycles = TotalRunningCyclesElapsed;

	return true;
}