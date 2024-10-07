#include "ScreenManagerSystem.h"

//Defaults If Not Provided.
uint16_t ResolutionWidth = 1080;
uint16_t ResolutionHeight = 1920;
float AspectRatio = static_cast<float>(ResolutionWidth) / static_cast<float>(ResolutionHeight);

//We should never pass in zero, but if for some odd reason we do... ASPECT RATIO won't cause a crash. 
void ScreenManagerSystem::UpdateScreenParameters(const uint16_t resolution_width, const  uint16_t resolution_height)
{
	ResolutionWidth = resolution_width;
	ResolutionHeight = resolution_height;

	if (ResolutionWidth != 0 && ResolutionHeight != 0)
	{
		AspectRatio = static_cast<float>(resolution_width) / static_cast<float>(resolution_height);
	}
	else
	{
		AspectRatio = 0.0f;
	}
}

uint16_t ScreenManagerSystem::GetScreenWidth()
{
	return ResolutionWidth;
}

uint16_t ScreenManagerSystem::GetScreenHeight()
{
	return ResolutionHeight;
}

float ScreenManagerSystem::GetScreenAspectRatio()
{
	return AspectRatio;
}