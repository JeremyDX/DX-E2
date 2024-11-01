#include "CameraEngine.h"

#include "Animation.h"
#include "Constants.h"
#include "ContentLoader.h"
#include "Engine.h"
#include "GameTime.h"
#include "ScreenManagerSystem.h"
#include "XGameInput.h"
#include "XModelMesh.h"
#include "BinaryCacheLoader.h"
#include "LandscapeSystems.h"

#include <cstdio>
#include <DirectXMath.h>

void UpdateCameraMatrixAndShaderResources();

constexpr float CONTROLLER_MAX_RANGE = 32767;
constexpr float CONTROLLER_VALUE_SCALING = 1.0f / CONTROLLER_MAX_RANGE; // 0.00003125;
constexpr double HALF_DEGREE_AS_RADIANS = 0.0087266462599;
constexpr double ONE_DEGREE_AS_RADIANS = 0.01745329251;
constexpr double NINETY_DEGREES_AS_RADIANS = 1.57079632679;

DirectX::XMFLOAT4X4 ViewMatrix;

Vector3D CameraUpVector;
Vector3D CameraForwardVector;
Vector3D CameraRightVector;
Vector3D CameraPosition;

float ControllerDeadzoneOffset = 0.0f;
float ControllerDeadzoneNormalizedMultiplier = 1.0f;

float CameraYaw;
float CameraPitch;
float CameraRoll;
float DistanceTraveled = 0;
float CurrentFov = 90.0f;
float HeightInMeters = 1.67f;

//Used for situations where we need to retain the values while we do actions so we can either A. Revert back to them or B. Lock the Camera to a specific direction.
float CameraYawSnapshot;
float CameraYawReturnRate;

uint8_t CameraSensitivityInternalModifier = 1000;

void CameraEngine::PreInitialize()
{
	D3D11_BUFFER_DESC BufferDescription = { 0 };
	BufferDescription.Usage = D3D11_USAGE_DEFAULT;
	BufferDescription.ByteWidth = 192U;
	BufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	Engine::device->CreateBuffer(&BufferDescription, nullptr, &BinaryCacheLoader::PerCameraChangeConstBuffer);

	Engine::context->VSSetConstantBuffers(1, 1, BinaryCacheLoader::PerCameraChangeConstBuffer.GetAddressOf());
	Engine::context->PSSetConstantBuffers(1, 1, BinaryCacheLoader::PerCameraChangeConstBuffer.GetAddressOf());
}

void UpdateCameraMatrixAndShaderResources()
{
	const float HorizontalFOV = CurrentFov * ONE_DEGREE_AS_RADIANS;  // 90 degrees in radians
	float AspectRatio = ScreenManagerSystem::GetScreenAspectRatio();
	float ResultFOV = static_cast<float>(2 * atan(tan(HorizontalFOV / 2) / AspectRatio));

	// Create the projection matrix with the calculated vertical FOV
	DirectX::XMMATRIX PROJECTION_MATRIX = DirectX::XMMatrixPerspectiveFovLH(ResultFOV, AspectRatio, 0.1F, 300.0F);

	PerCameraChangeConstBufferStruct PerCameraChangeData = { 0 };

	DirectX::XMMATRIX MultipliedMatrix = DirectX::XMLoadFloat4x4(&ViewMatrix) * PROJECTION_MATRIX;

	// Calculate the inverse of the combined matrix
	DirectX::XMMATRIX InverseMatrix = DirectX::XMMatrixInverse(nullptr, MultipliedMatrix);

	//ViewMatrix *= PROJECTION_MATRIX;
	// Store the final result
	DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&PerCameraChangeData.ViewMatrix), MultipliedMatrix);
	DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&PerCameraChangeData.InverseViewMatrix), InverseMatrix);
	PerCameraChangeData.CameraPosition = CameraPosition;
	PerCameraChangeData.Yaw = CameraYaw;
	PerCameraChangeData.CameraForwardVector = CameraForwardVector;
	PerCameraChangeData.Pitch = CameraPitch;
	PerCameraChangeData.CameraRightVector = CameraRightVector;
	PerCameraChangeData.Roll = CameraRoll;
	PerCameraChangeData.CameraUpVector = CameraUpVector;

	Engine::context->UpdateSubresource(BinaryCacheLoader::PerCameraChangeConstBuffer.Get(), 0, 0, &PerCameraChangeData, 0, 0);
}

constexpr void CalculateDeadzoneModifiers(int ChangedDeadZonePercent)
{
	//Reduce % to 0.0 to 0.9 then multiplies by 32000.
	ControllerDeadzoneOffset = ChangedDeadZonePercent * 0.01f * CONTROLLER_MAX_RANGE;
	ControllerDeadzoneNormalizedMultiplier = CONTROLLER_MAX_RANGE / (CONTROLLER_MAX_RANGE - ControllerDeadzoneOffset);
}

void CameraEngine::ResetPrimaryCameraMatrix(float FACE_DIRECTION)
{
	CameraYaw = FACE_DIRECTION;
	CameraPitch = 0;
	CameraRoll = 0;

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
	ViewMatrix._14 = 0.0F;
	ViewMatrix._24 = 0.0F;
	ViewMatrix._34 = 0.0F;
	ViewMatrix._44 = 1.0F;

	UpdateCameraMatrixAndShaderResources();
}

void CameraEngine::BuildPrimaryCameraMatrix()
{
	//Right Vector.
	ViewMatrix._11 = CameraRightVector.X; //Sin(Turn)
	ViewMatrix._21 = 0.0F;     //Always Flat at 0.0F.
	ViewMatrix._31 = CameraRightVector.Z; //Cos(Turn)

	//Up Vector.
	ViewMatrix._12 = CameraUpVector.X * CameraForwardVector.X;   //Sin(Look) * Sin(Turn)
	ViewMatrix._22 = CameraUpVector.Z;                //Cos(Look)
	ViewMatrix._32 = CameraUpVector.X * CameraForwardVector.Z;   //Sin(Look) * Cos(Turn)

	//Forward Vector.
	ViewMatrix._13 = CameraUpVector.Z * CameraForwardVector.X; //Cos(Look) * Sin(Turn)
	ViewMatrix._23 = -CameraUpVector.X;              //Inverse of Sin(Look)
	ViewMatrix._33 = CameraUpVector.Z * CameraForwardVector.Z; //Cos(Look) * Cos(Turn)

	float GroundHeight = LandscapeSystems::GetCurrentHeightAtLocation(CameraPosition.X, CameraPosition.Z);

	float HeightInMeters = 1.67f;

	const float CameraXOffset = (CameraPosition.X + CameraForwardVector.X * 0.5f * HeightInMeters);
	const float CameraZOffset = (CameraPosition.Z + CameraForwardVector.Z * 0.5f * HeightInMeters);
	const float CameraHeight = CameraPosition.Y + GroundHeight + HeightInMeters;

	//Dot Product (Position, Right Vector).
	ViewMatrix._41 = -(CameraXOffset * ViewMatrix._11 + CameraHeight * ViewMatrix._21 + CameraZOffset * ViewMatrix._31);

	//Dot Product (Position, Up Vector).
	ViewMatrix._42 = -(CameraXOffset * ViewMatrix._12 + CameraHeight * ViewMatrix._22 + CameraZOffset * ViewMatrix._32);

	//Dot Product (Position, Forward Vector).
	ViewMatrix._43 = -(CameraXOffset * ViewMatrix._13 + CameraHeight * ViewMatrix._23 + CameraZOffset * ViewMatrix._33);
}

int GetCameraSensitivity()
{
	return CameraSensitivityInternalModifier;
}

/*
* Ends up returning a value ranging from deadzone minimum to 1.0f * DeltaFrame.
*/ 
bool CanSmoothAndNormalizeJoystickValue(float &Value, const float DeltaFrame)
{
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


float DistanceBetweenVectors(const Vector3D& A, const Vector3D& B) {
	float dx = B.X - A.X;
	float dy = B.Y - A.Y;
	float dz = B.Z - A.Z;
	return sqrtf(dx * dx + dy * dy + dz * dz);
}

bool bIsSprinting = false;

bool CameraEngine::PrimaryCameraUpdatedLookAt()
{
	//Time Slice from 0.0f to 0.0167 in event we're running slow. This is to prevent using Lag as a movement exploit and requires steady Target FPS amount.
	const float DeltaFrame = min(GameTime::GetDeltaElapsedTime(), GameTime::GetFrameTickLimit());

	//Get Controller Strength's / WASD For Camera Movement, Ranges from 0 (0%) to 32000 (100%)
	float CameraRightStrength = static_cast<float>(XGameInput::GetRightMovementStrength());
	float CameraForwardStrength = static_cast<float>(XGameInput::GetForwardMovementStrength());

	//Get Controller Strengths / Mouse Changes For Turning. 
	float CameraTurnStrength = static_cast<float>(XGameInput::GetRightStickX());
	float CameraLookStrength = static_cast<float>(XGameInput::GetRightStickY());

	bool CameraNeedsUpdate = false;

	if (XGameInput::ActionWasInitiated(DirectButtonActionsEnum::HOLD_LOOK))
	{
		CameraYawSnapshot = CameraYaw;
		CameraYawReturnRate = 0.0f;
	}

	if (XGameInput::ActionHasEnded(DirectButtonActionsEnum::HOLD_LOOK))
	{
		CameraYawReturnRate = (CameraYawSnapshot - CameraYaw) / 2.0f;
	}

	Vector3D MovementForwardVector;

	if (CanSmoothAndNormalizeJoystickValue(CameraTurnStrength, DeltaFrame))
	{
		const float TurnCalculation = CameraTurnStrength * GetCameraSensitivity();

		CameraYaw += TurnCalculation;

		if (XGameInput::ActionIsCurrentlyActive(DirectButtonActionsEnum::HOLD_LOOK))
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

			float RotationAsRadians = CameraYawSnapshot * ONE_DEGREE_AS_RADIANS;

			MovementForwardVector.X = static_cast<float>(sin(RotationAsRadians));
			MovementForwardVector.Z = static_cast<float>(cos(RotationAsRadians));
		} 
		else 
		{

		}

		double RotationAsRadians = CameraYaw * ONE_DEGREE_AS_RADIANS;

		CameraForwardVector.X = static_cast<float>(sin(RotationAsRadians));
		CameraForwardVector.Z = static_cast<float>(cos(RotationAsRadians));

		CameraRightVector.X = CameraForwardVector.Z;
		CameraRightVector.Z = -CameraForwardVector.X;

		CameraNeedsUpdate = true;
	}

	if (CanSmoothAndNormalizeJoystickValue(CameraLookStrength, DeltaFrame))
	{
		const float LookCalculation = CameraLookStrength * GetCameraSensitivity();

		CameraPitch += LookCalculation;

		constexpr float PitchLimit = 89.5f;

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

	float MPH_SPEED = 5.0f / 0.8f;

	if (CanSmoothAndNormalizeJoystickValue(CameraForwardStrength, DeltaFrame))
	{
		if (XGameInput::ActionWasInitiated(DirectButtonActionsEnum::SPRINT))
		{
			bIsSprinting = !bIsSprinting;
		}

		if (bIsSprinting)
		{
			MPH_SPEED *= 2.5f;
		}

		CameraPosition.X += CameraForwardVector.X * CameraForwardStrength * MPH_SPEED;
		CameraPosition.Z += CameraForwardVector.Z * CameraForwardStrength * MPH_SPEED;

		CameraNeedsUpdate = true;
	} 
	else 
	{
		bIsSprinting = false;
	}

	if (CanSmoothAndNormalizeJoystickValue(CameraRightStrength, DeltaFrame))
	{
		CameraPosition.X += CameraRightVector.X * CameraRightStrength * MPH_SPEED;
		CameraPosition.Z += CameraRightVector.Z * CameraRightStrength * MPH_SPEED;
		CameraNeedsUpdate = true;
	}

	if (XGameInput::ActionIsCurrentlyActive(DirectButtonActionsEnum::JUMPING))
	{
		HeightInMeters += DeltaFrame * 5.0f;
		CameraNeedsUpdate = true;
	}

	if (XGameInput::ActionIsCurrentlyActive(DirectButtonActionsEnum::CROUCHING))
	{
		HeightInMeters -= DeltaFrame * 5.0f;
		CameraNeedsUpdate = true;
	}

	if (CameraNeedsUpdate)
	{
		BuildPrimaryCameraMatrix();
		UpdateCameraMatrixAndShaderResources();
	}
	
	return CameraNeedsUpdate;
}


void CameraEngine::GetDebugString(char* Out_Chars, int CharLength)
{
	sprintf_s(Out_Chars, CharLength, "FwdVec: [X: %f, Z: %f], Pos: [X: %f Y: %f Z: %f] Pitch: %f, Yaw: %f, Meters/s: %f", CameraForwardVector.X, CameraForwardVector.Z, CameraPosition.X, CameraPosition.Y, CameraPosition.Z, CameraPitch, CameraYaw, DistanceTraveled);
}
