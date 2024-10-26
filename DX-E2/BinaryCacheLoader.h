#pragma once

struct VertexShaderCache
{

};

struct PixelShaderCache
{

};

class BinaryCacheLoader
{
	public:

		static void LoadShaders();
		static void UseShaders(const int VertexShaderIndex, const int PixelShaderIndex);
};