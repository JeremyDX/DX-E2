#include "CameraEngine.h"

#include "Animation.h"
#include "Constants.h"
#include "ContentLoader.h"
#include "Engine.h"
#include "GameTime.h"
#include "ScreenManagerSystem.h"
#include "XGameInput.h"
#include "XModelMesh.h"

#include <cstdio>
#include <DirectXMath.h>

void CreateFinalMatrixResult();

constexpr float CONTROLLER_MAX_RANGE = 32767;
constexpr float CONTROLLER_VALUE_SCALING = 1.0f / CONTROLLER_MAX_RANGE; // 0.00003125;
constexpr double HALF_DEGREE_AS_RADIANS = 0.0087266462599;
constexpr double ONE_DEGREE_AS_RADIANS = 0.01745329251;
constexpr double NINETY_DEGREES_AS_RADIANS = 1.57079632679;

DirectX::XMFLOAT4X4 CameraEngine::final_result;
DirectX::XMFLOAT4X4 camera_matrix;

Vector3D CameraUpVector;
Vector3D CameraForwardVector;
Vector3D CameraRightVector;
Vector3D CameraPosition;

float ControllerDeadzoneOffset = 0.0f;
float ControllerDeadzoneNormalizedMultiplier = 1.0f;

float CameraYaw;
float CameraPitch;
float CameraRoll;

//Used for situations where we need to retain the values while we do actions so we can either A. Revert back to them or B. Lock the Camera to a specific direction.
float CameraYawSnapshot;
float CameraYawReturnRate;

uint8_t CameraSensitivityInternalModifier = 10;


int GetCameraSensitivity()
{
	return CameraSensitivityInternalModifier;
}

constexpr void CalculateDeadzoneModifiers(int ChangedDeadZonePercent)
{
	//Reduce % to 0.0 to 0.9 then multiplies by 32000.
	ControllerDeadzoneOffset = ChangedDeadZonePercent * 0.01f * CONTROLLER_MAX_RANGE;
	ControllerDeadzoneNormalizedMultiplier = CONTROLLER_MAX_RANGE / (CONTROLLER_MAX_RANGE - ControllerDeadzoneOffset);
}

/*
* Ends up returning a value ranging from deadzone minimum to 1.0f * DeltaFrame.
*/ 
bool CanSmoothAndNormalizeJoystickValue(float &Value, const float DeltaFrame)
{
	if (Value > 16300 && Value < 16400)
	{
		int junk2 = 0;
	}

	if (Value < 0)
	{
		if (Value >= -ControllerDeadzoneOffset)
		{
			return false;
		}

		Value += ControllerDeadzoneOffset + 1;
		Value = Value * ControllerDeadzoneNormalizedMultiplier;
		Value = floor((Value * Value * -CONTROLLER_VALUE_SCALING) + 0.5f);
	} 
	else 
	{ 
		if (Value <= ControllerDeadzoneOffset)
		{
			return false;
		}

		Value -= ControllerDeadzoneOffset;
		Value = Value * ControllerDeadzoneNormalizedMultiplier;
		Value = floor((Value * Value * CONTROLLER_VALUE_SCALING) + 0.5f);
	}

	Value *= DeltaFrame;
	Value *= CONTROLLER_VALUE_SCALING;

	return true;
}

/*
void CameraEngine::UpdatePromptView()
{
	if (prompt_viewport._1 != -1)
	{
		if (prompt_viewport._2 > prompt_viewport._3)
		{
			if (rotation_data._2 > prompt_viewport._2 || rotation_data._2 < prompt_viewport._3)
			{
				test._1 = prompt_viewport._1;
			}
		}
		else
		{
			if (rotation_data._2 > prompt_viewport._2 && rotation_data._2 < prompt_viewport._3)
			{
				test._1 = prompt_viewport._1;
			}
		}
	}
}
*/

/*
uint64_t BaseAnimationFrameIndex = 0LLU;
uint64_t CurrentAnimationFrameIndex = 0LLU;
uint16_t AnimationIndexId = 0;
Float2 LockedForward = { 0 };
Float2 LockedRight = { 0 };
*/

bool CameraEngine::PrimaryCameraUpdatedLookAt()
{
	//Time Slice from 0.0f to 0.0167 in event we're running slow. This is to prevent using Lag as a movement exploit and requires steady Target FPS amount.
	const float DeltaFrame = min(GameTime::GetFrameTickDelta(), GameTime::GetFrameTickLimit());

	//Get Controller Strength's For Camera Movement & Camera Turning., Ranges from 0 (0%) to 32000 (100%)
	float CameraRightStrength = static_cast<float>(XGameInput::GetLeftStickX());
	float CameraForwardStrength = static_cast<float>(XGameInput::GetLeftStickY());
	float CameraTurnStrength = static_cast<float>(XGameInput::GetRightStickX());
	float CameraLookStrength = static_cast<float>(XGameInput::GetRightStickY());

	bool CameraNeedsUpdate = false;

	if (XGameInput::AnyOfTheseButtonsArePressed(XBOX_CONTROLLER::LEFT_BUMPER))
	{
		CameraYawSnapshot = CameraYaw;
		CameraYawReturnRate = 0.0f;
	}

	if (XGameInput::AnyOfTheseButtonsAreReleased(XBOX_CONTROLLER::LEFT_BUMPER))
	{
		CameraYawReturnRate = (CameraYawSnapshot - CameraYaw) / 2.0f;
	}

	if (CanSmoothAndNormalizeJoystickValue(CameraTurnStrength, DeltaFrame))
	{
		const float TurnCalculation = CameraTurnStrength * GetCameraSensitivity();

		CameraYaw += TurnCalculation;

		if (XGameInput::AnyOfTheseButtonsAreHolding(XBOX_CONTROLLER::LEFT_BUMPER))
		{
			if (CameraYaw > CameraYawSnapshot)
			{
				float YawDifference = CameraYaw - CameraYawSnapshot;
				if (YawDifference > 140.0f)
				{
					CameraYaw = CameraYawSnapshot + 140.0f;
				}
			} 
			else
			{
				float YawDifference = CameraYawSnapshot - CameraYaw;
				if (YawDifference > 140.0f)
				{
					CameraYaw = CameraYawSnapshot - 140.0f;
				}
			}
		}

		double RotationAsRadians = CameraYaw * ONE_DEGREE_AS_RADIANS;
		double PitchAsRadians = (90 - fabs(CameraPitch)) * ONE_DEGREE_AS_RADIANS;

		CameraForwardVector.X = static_cast<float>(sin(RotationAsRadians));
		CameraForwardVector.Z = static_cast<float>(cos(RotationAsRadians));
		CameraRightVector.X = CameraForwardVector.Z;
		CameraRightVector.Z = -CameraForwardVector.X;
		//CameraPosition.X -= CameraRightVector.X * DeltaFrame * 0.01;
		//CameraPosition.Z -= CameraRightVector.Z * DeltaFrame * 0.01;


		// Adjust 0.005 based on the pitch factor
		//float AdjustedValue = 0.001f * fabs(CameraPitch) * CameraForwardVector.Z; // Reduce as pitch approaches -90 or 90


		//CameraPosition.X -= CameraForwardVector.Z * AdjustedValue * DeltaFrame;
		//CameraPosition.Z += CameraForwardVector.X * AdjustedValue * DeltaFrame;

		CameraNeedsUpdate = true;
	}

	if (CanSmoothAndNormalizeJoystickValue(CameraLookStrength, DeltaFrame))
	{
		const float LookCalculation = CameraLookStrength * GetCameraSensitivity();

		CameraPitch += LookCalculation;

		constexpr float PitchLimit = 85.0f;

		if (CameraPitch > PitchLimit)
		{
			CameraPitch = PitchLimit;
		} 
		else if (CameraPitch < -PitchLimit)
		{
			CameraPitch = -PitchLimit;
		}

		double RotationAsRadians = CameraPitch * ONE_DEGREE_AS_RADIANS;

		CameraUpVector.X = -static_cast<float>(sin(RotationAsRadians));
		CameraUpVector.Z = static_cast<float>(cos(RotationAsRadians));

		CameraNeedsUpdate = true;
	}

	if (CanSmoothAndNormalizeJoystickValue(CameraForwardStrength, DeltaFrame))
	{
		CameraPosition.X += CameraForwardVector.X * CameraForwardStrength * 0.075;// *10.6761565836;
		CameraPosition.Z += CameraForwardVector.Z * CameraForwardStrength * 0.075;
		CameraNeedsUpdate = true;
	}

	if (CanSmoothAndNormalizeJoystickValue(CameraRightStrength, DeltaFrame))
	{
		CameraPosition.X += CameraRightVector.X * CameraRightStrength * 0.075;
		CameraPosition.Z += CameraRightVector.Z * CameraRightStrength * 0.075;
		CameraNeedsUpdate = true;
	}

	if (CameraNeedsUpdate)
	{

		BuildPrimaryCameraMatrix();
		CreateFinalMatrixResult();
	}

	return CameraNeedsUpdate;
}

/*
void CreateFinalMatrixResult()
{
	static const DirectX::XMMATRIX PROJECTION_MATRIX = DirectX::XMMatrixPerspectiveFovLH((float)(45 * ONE_DEGREE_AS_RADIANS), ScreenManagerSystem::GetScreenAspectRatio(), 0.1F, 300.0F);
	DirectX::XMStoreFloat4x4(&CameraEngine::final_result, DirectX::XMLoadFloat4x4(&camera_matrix) * PROJECTION_MATRIX);
}
*/

void CreateFinalMatrixResult()
{
	// Define the horizontal FOV in degrees
	float horizontalFOVDegrees = 90.0f; // Set your desired horizontal FOV here
	float aspectRatio = ScreenManagerSystem::GetScreenAspectRatio(); // Get the aspect ratio of your window

	// Convert horizontal FOV to vertical FOV
	float horizontalFOVRadians = horizontalFOVDegrees * ONE_DEGREE_AS_RADIANS;
	float verticalFOVRadians = 2 * atan(tan(horizontalFOVRadians / 2) / aspectRatio);

	// Create the projection matrix with the calculated vertical FOV
	static const DirectX::XMMATRIX PROJECTION_MATRIX = DirectX::XMMatrixPerspectiveFovLH(verticalFOVRadians, aspectRatio, 0.1F, 300.0F);

	// Store the final result
	DirectX::XMStoreFloat4x4(&CameraEngine::final_result, DirectX::XMLoadFloat4x4(&camera_matrix) * PROJECTION_MATRIX);
}

void CameraEngine::ResetPrimaryCameraMatrix(float FACE_DIRECTION)
{
	CameraYaw = FACE_DIRECTION;

	CalculateDeadzoneModifiers(10);

	CameraPosition.X = 1.0F;
	CameraPosition.Y = 0.0F;
	CameraPosition.Z = 1.0F;

	//rotation_data._1 = 0;
	//rotation_data._2 = rotation_data._3 = FACE_DIRECTION * 32000;
	//rotation_data._4 = 0;

	//prompt_viewport._1 = -1;
	//prompt_viewport._2 =  0;
	//prompt_viewport._3 =  0;

	CameraUpVector.X = 0.0f;
	CameraUpVector.Z = 1.0f;

	double rotation = FACE_DIRECTION * ONE_DEGREE_AS_RADIANS;

	CameraForwardVector.X = (float)sin(rotation);
	CameraForwardVector.Z = (float)cos(rotation);

	rotation += NINETY_DEGREES_AS_RADIANS;

	CameraRightVector.X = (float)sin(rotation);
	CameraRightVector.Z = (float)cos(rotation);

	/*
	float SinH = CameraForwardVector.X * 0.125F;
	float CosH = CameraForwardVector.Z * 0.125F;
	float SinW = CameraForwardVector.X * 0.250F;
	float CosW = CameraForwardVector.Z * 0.250F;

	blocking_value[0]._1 = (CosW + SinH);
	blocking_value[0]._2 = (-SinW + CosH);
	blocking_value[1]._1 = (-CosW + SinH);
	blocking_value[1]._2 = (SinW + CosH);

	blocking_value[2]._1 = -blocking_value[0]._1;
	blocking_value[2]._2 = -blocking_value[0]._2;
	blocking_value[3]._1 = -blocking_value[1]._1;
	blocking_value[3]._2 = -blocking_value[1]._2;
	*/

	BuildPrimaryCameraMatrix();

	//Not Sure Unused atm. 
	camera_matrix._14 = 0.0F;
	camera_matrix._24 = 0.0F;
	camera_matrix._34 = 0.0F;
	camera_matrix._44 = 1.0F;

	CreateFinalMatrixResult();
}

void CameraEngine::BuildPrimaryCameraMatrix()
{
	//Right Vector.
	camera_matrix._11 = CameraRightVector.X; //Sin(Turn)
	camera_matrix._21 = 0.0F;     //Always Flat at 0.0F.
	camera_matrix._31 = CameraRightVector.Z; //Cos(Turn)

	//Up Vector.
	camera_matrix._12 = CameraUpVector.X * CameraForwardVector.X;   //Sin(Look) * Sin(Turn)
	camera_matrix._22 = CameraUpVector.Z;                //Cos(Look)
	camera_matrix._32 = CameraUpVector.X * CameraForwardVector.Z;   //Sin(Look) * Cos(Turn)

	//Forward Vector.
	camera_matrix._13 = CameraUpVector.Z * CameraForwardVector.X; //Cos(Look) * Sin(Turn)
	camera_matrix._23 = -CameraUpVector.X;              //Inverse of Sin(Look)
	camera_matrix._33 = CameraUpVector.Z * CameraForwardVector.Z; //Cos(Look) * Cos(Turn)

	const float CameraXOffset = (CameraPosition.X + CameraForwardVector.X);
	const float CameraZOffset = (CameraPosition.Z + CameraForwardVector.Z);
	const float CameraHeight = CameraPosition.Y + 1;

	//Dot Product (Position, Right Vector).
	camera_matrix._41 = -(CameraXOffset * camera_matrix._11 + CameraHeight * camera_matrix._21 + CameraZOffset * camera_matrix._31);

	//Dot Product (Position, Up Vector).
	camera_matrix._42 = -(CameraXOffset * camera_matrix._12 + CameraHeight * camera_matrix._22 + CameraZOffset * camera_matrix._32);

	//Dot Product (Position, Forward Vector).
	camera_matrix._43 = -(CameraXOffset * camera_matrix._13 + CameraHeight * camera_matrix._23 + CameraZOffset * camera_matrix._33);
}

void CameraEngine::GetDebugString(char* Out_Chars, int CharLength)
{
	sprintf_s(Out_Chars, CharLength, "FwdVec: [X: %f, Z: %f], Pos: [X: %f Y: %f Z: %f] Pitch: %f, Yaw: %f", CameraForwardVector.X, CameraForwardVector.Z, CameraPosition.X, CameraPosition.Y, CameraPosition.Z, CameraPitch, CameraYaw);
}
