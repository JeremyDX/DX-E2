Texture2D HeightMap : register(t0);
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

struct VertexShaderOutput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

//target vs_4_0_level_9_3

VertexShaderOutput main(int packed_position : PACKED_INTEGER)
{
	VertexShaderOutput output;

	float zPos = floor(packed_position.x / 2048.0f); // Extract Z
	float xPos = packed_position.x - (zPos * 2048.0f); // Extract X

	// Initialize position as float4
	float4 position = float4(xPos, 0.0f, zPos, 1.0f);

	float size = 0.1f;

	position.x *= size * CameraPosition.x;
	position.z *= size;

	output.texcoord.x = position.x;
	output.texcoord.y = position.z;

	float Height = HeightMap.SampleLevel(ss, output.texcoord * 0.0009765625f, 0).r;
	position.y = Height * 200.2;

	output.position = mul(ViewProjectionMatrix, position);

	return output;
}