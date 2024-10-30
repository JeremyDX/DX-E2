Texture2D Texture : register(t0);
Texture2D AlphaMap : register(t1);

SamplerState ss : register(s0);

cbuffer PerFrameConstants : register(b0)
{
	float DeltaTime;
	float ElapsedTime;
	int CurrentTargetFrameIndex;
	float UNUSED;
};

cbuffer PerFrameConstants : register(b1)
{
	float4x4 ViewProjectionMatrix;
	float4x4 InverseViewProjectionMatrix;
	float3 CameraPosition;
	float CameraYaw;
	float3 CameraForwardVector;
	float CameraPitch;
	float3 CameraRightVector;
	float CameraRoll;
	float3 CameraUpVector;
	float UNUSED2;
};

// Pixel Shader example
float4 main(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD/*, float3 worldPos : TEXCOORD1*/) : SV_TARGET
{
/*
	float3 FoVal = CameraForwardVector;
	FoVal.x *= CameraUpVector.z * 0.5f;
	FoVal.z *= CameraUpVector.z * 0.5f;

	float3 CamValue = CameraPosition + (FoVal * 4.0f);

	float diffX = CamValue.x - worldPos.x;
	float diffZ = CamValue.z - worldPos.z;

	int distance = 200.0f;

	float LowerBounds = (diffX * diffX + diffZ * diffZ);
	float UpperBounds = distance * distance;

	if (LowerBounds < UpperBounds)
	{
		float Amount = (1.0f - saturate((LowerBounds / UpperBounds) * 1.25f));

		float Alpha = AlphaMap.Sample(ss, texcoord).r;

		texcoord *= 1024;

		float4 ColorA = Texture.Sample(ss, texcoord);

		//texcoord *= 256;

		float4 DarkGreen = float4(0, 0.6, 0, 1); // Dark green color
		float4 DarkTan = float4(0.678, 0.569, 0.482, 1.0);

		return lerp(ColorA, DarkTan * ColorA, Alpha); //lerp(ColorA, DarkGreen, Alpha);
	}

	texcoord *= 256;
*/

	float4 ColorA = Texture.Sample(ss, texcoord);

	return ColorA;
}