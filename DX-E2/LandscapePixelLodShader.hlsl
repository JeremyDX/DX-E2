Texture2D Texture;
SamplerState ss;

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	return Texture.Sample(ss, texcoord);
}