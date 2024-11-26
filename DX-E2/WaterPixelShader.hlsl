Texture2D Texture : register(t0);
Texture2D AlphaMap : register(t1);

SamplerState ss : register(s0);

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

float4 main(float4 position : SV_POSITION, float4 color2 : COLOR, float2 texcoord : TEXCOORD, float3 worldPos : TEXCOORD1) : SV_TARGET
{
/*
	float diffX = CameraPosition.x - worldPos.x;
	float diffZ = CameraPosition.z - worldPos.z;
	float Distance = diffX * diffX + diffZ * diffZ;
	const float MaxDistance = 1024.0f * 1024.0f * 0.075;
	float AlphaValue = saturate(Distance / MaxDistance);
	AlphaValue;

	float4 GroundTexture = Texture.Sample(ss, texcoord);
	float2 WholeMapCoords = texcoord / 128;
	float HeightColor = AlphaMap.Sample(ss, WholeMapCoords).r;

	float4 ResultColor = 1;
	float NoiseColor = 1.0f - lerp(0, HeightColor, AlphaValue);

	ResultColor.rgb = lerp(0, GroundTexture.rgb, NoiseColor);

	float3 color_adjustment = float3(2.0f, 1.7f, 1.4f);

	ResultColor.rgb = saturate(ResultColor.rgb * color_adjustment);
*/
	float4 color_adjustment = float4(0.1f, 0.4, 1.0f, 1.f);

	return color_adjustment;
}