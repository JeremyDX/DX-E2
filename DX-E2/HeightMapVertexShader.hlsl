cbuffer HLSLBuffer : register(b0)
{
	float4x4 view_matrix;
};

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;        // texture coordinates
};

Texture2D colorMap : register(t0);
SamplerState colorSampler : register(s0);

//target vs_4_0_level_9_3

VOut main(float4 position : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD)
{
	VOut output;

	float HeightColor = colorMap.SampleLevel(colorSampler, float2(position.x * 0.0125, position.z * 0.0125), 0).r;

	//float4 colors = colorMap.SampleLevel(colorSampler, float4(position.x * 0.001, position.z * 0.001, 0, 0), 0);
	//position.y = colors.x * 128;

	position.y -= 0.0f;//HeightColor;
	output.position = mul(view_matrix, position);
	///output.position.y + 0.5f;
	int base1 = (int)(color.r / 256); //0-255
	int base2 = ((int)color.r) - (base1 * 256);
	output.color.r = (base1 / 255.0f);
	output.color.g = color.g;
	output.color.b = color.b;

	output.color.a = (base2 / 255.0f);

	output.texcoord = texcoord;    // set the texture coordinates, unmodified
	return output;
}