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


/*
	float CurrentLod = AlphaMap.CalculateLevelOfDetail(ss, texcoord);
	CurrentLod = clamp(CurrentLod, 0, 8.0f) / 8.0f;

	float2 WholeMapCoords = texcoord / 64;

	float HeightColor = AlphaMap.Sample(ss, WholeMapCoords).r * CurrentLod * CurrentLod;
	HeightColor = saturate(HeightColor); 
	float4 BrownColor = float4(0.075, 0.04, 0.01, 1.0f);

	float4 TextureColor = Texture.Sample(ss, texcoord);
	float3 FinalColor = lerp(TextureColor.rgb, BrownColor.rgb, HeightColor * 1.5);
	
	TextureColor.rgb *= FinalColor;

	TextureColor.rgb = FinalColor;
	TextureColor.a = 1.0f;
*/
	//float4 TextureColor = Texture.Sample(ss, texcoord);
	//return TextureColor;

	float diffX = CameraPosition.x - worldPos.x;
	float diffZ = CameraPosition.z - worldPos.z;
	float Distance = diffX * diffX + diffZ * diffZ;
	const float MaxDistance = 1024.0f * 1024.0f * 0.075;
	float AlphaValue = saturate(Distance / MaxDistance);
	AlphaValue;

	float4 GroundTexture = Texture.Sample(ss, texcoord / 4);
	float2 WholeMapCoords = texcoord / 128;
	float HeightColor = AlphaMap.Sample(ss, WholeMapCoords).r;
/*
	float4 GroundTexture = Texture.Sample(ss, texcoord);
	float3 BrownTest = lerp(GroundTexture.rgb, 1, saturate(HeightColor));
*/
	float4 ResultColor = 1;
	float NoiseColor = 1.0f - lerp(0, HeightColor, AlphaValue);

	ResultColor.rgb = lerp(0, GroundTexture.rgb, NoiseColor);

	float3 color_adjustment = float3(2.0f, 1.7f, 1.4f);

	ResultColor.rgb = saturate(ResultColor.rgb * color_adjustment);

	return ResultColor;//TextureColor;// * ColorA;
}