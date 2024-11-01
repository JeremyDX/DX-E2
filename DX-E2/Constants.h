#pragma once

#include <d3d11.h>
#include <cstdint>

struct Vector3D
{
	float X, Y, Z;
};

struct Float2
{
	float _1, _2;
};

struct Float3
{
	float _1, _2, _3;
};

struct Float4
{
	float _1, _2, _3, _4;
};

struct Byte2
{
	int8_t _1, _2;
};

struct Byte3
{
	int8_t _1, _2, _3;
};

struct Byte4
{
	int8_t _1, _2, _3, _4;
};

struct Short2
{
	int16_t _1, _2;
};

struct Short3
{
	int16_t _1, _2, _3;
};

struct Short4
{
	int16_t _1, _2, _3, _4;
};

struct Int2
{
	int32_t _1, _2;
};

struct Int3
{
	int32_t _1, _2, _3;
};

struct Int4
{
	int32_t _1, _2, _3, _4;
};

struct Long2
{
	int64_t _1, _2;
};

struct Long3
{
	int64_t _1, _2, _3;
};

struct Long4
{
	int64_t _1, _2, _3, _4;
};

struct VertexPackedInteger
{
	int PackedInteger;
};

struct Vertex20Byte
{
	float _X, _Y, _Z;
	float _U, _V;
};

struct Vertex32Byte
{
	float _X, _Y, _Z;
	float _1, _2, _3;
	float _U, _V;
};

struct Vertex44Byte
{
	float _X, _Y, _Z;
	float _1, _2, _3;
	float _6, _7, _8;
	float _U, _V;
};

struct Constants
{
	static const D3D11_INPUT_ELEMENT_DESC Layout_PackedInt[];
	static const D3D11_INPUT_ELEMENT_DESC Layout_Byte20[];
	static const D3D11_INPUT_ELEMENT_DESC Layout_Byte1[];
	static const D3D11_INPUT_ELEMENT_DESC Layout_Byte32[];
	static const D3D11_INPUT_ELEMENT_DESC Layout_Byte44[];
};