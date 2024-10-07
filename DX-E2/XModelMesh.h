#pragma once

#include "Constants.h"

class XModelMesh
{
	public:
		static     void LoadCollisionData();
		static     void CheckResolutionCollision(Float3 & ref, Float2 & move_vector, int result);
		static     int CheckBasicCollision(Float3 & ref, Float2 & move_vector, int view_rotation, Int3 & viewport);
		static     void TestValues();
		static     void LoadObjectDefintions();
		static int32_t CreateTexturedSquare(Vertex32Byte * verts, int offset, Float3 Col, int texture_width, int texture_height, int drawX, int drawY);
		static int32_t InsertObjectToMap(Vertex32Byte * verts, int & offset, int id, int xunits, int yunits, int zunits);
};