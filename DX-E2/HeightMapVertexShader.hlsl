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

VOut main(float4 position : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD)
{
	VOut output;

	output.texcoord.x = 0.00390625 * position.x * 0.25;
	output.texcoord.y = 0.00390625 * position.z * 0.25;

	float Height = HeightMap.SampleLevel(ss, output.texcoord, 0).r;

	position.y += Height * 100.0f;

	output.color.rgba = 1;
	output.worldPos = position.xyz;
	output.position = mul(view_matrix, position);

	return output;
}