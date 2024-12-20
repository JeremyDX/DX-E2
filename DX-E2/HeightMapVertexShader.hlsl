Texture2D HeightMap : register(t0);
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

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;        // texture coordinates
	float3 worldPos : TEXCOORD1;
};

//target vs_4_0_level_9_3

VOut main(int packed_position : PACKED_INTEGER)
{
	VOut output;

	float zPos = floor(packed_position.x / 2048.0f); // Extract Z
	float xPos = packed_position.x - (zPos * 2048.0f); // Extract X

	// Initialize position as float4
	float4 position = float4(xPos, 0.0f, zPos, 1.0f);

	position.x += (int)CameraPosition.x;
	position.z += (int)CameraPosition.z;

	output.texcoord.x = (0.0009765625 * position.x);
	output.texcoord.y = 1.0f - (0.0009765625 * position.z);

	saturate(output.texcoord);

	float Height = HeightMap.SampleLevel(ss, output.texcoord, 0).r;

	output.texcoord.x *= 256.0f * 16;
	output.texcoord.y *= 256.0f * 16;

	int intValue = (int)floor(xPos);
	int intValue2 = (int)floor(zPos);
	float3 color;

	color = float3(0.0f, 0.0f, 1.0f);

/*
	if (intValue == 0) {
		color = float3(1.0f, 0.0f, 0.0f);
	}
	if (intValue2 == 0) {
		color = float3(1.0f, 0.0f, 0.0f);
	}
	if (intValue == 1024) {
		color = float3(1.0f, 0.0f, 0.0f);
	}
	if (intValue2 == 1024) {
		color = float3(1.0f, 0.0f, 0.0f);
	}*/

	output.color.rgb = color;

	position.y = Height * 50.2;
	position.x -= 512;
	position.z -= 512;

	output.worldPos = position.xyz;
	output.position = mul(ViewProjectionMatrix, position);

	return output;
}