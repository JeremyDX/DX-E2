Texture2D HeightMap : register(t0);
SamplerState ss : register(s0);

cbuffer PerFrameConstants : register(b0)
{
	float DeltaTime;
	float ElapsedTime;
	int CurrentTargetFrameIndex;
	float UNUSED;
};

cbuffer CameraViewMatrix : register(b1)
{
	float4x4 view_matrix;
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

	output.texcoord.x = (0.00390625 * position.x * 0.25);
	output.texcoord.y = 1.0f - (0.00390625 * position.z * 0.25);

	float Height = HeightMap.SampleLevel(ss, output.texcoord, 0).r;

	output.texcoord.x *= 1024.0f;
	output.texcoord.y *= 1024.0f;
	

	position.y = 0.0f; //Height * 109.2f;

	output.color.rgba = 1;
	output.worldPos = position.xyz;
	output.position = mul(view_matrix, position);

	return output;
}