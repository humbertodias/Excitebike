#pragma once
#include "baseTypes.h"

typedef struct vector2_t
{
	float x;
	float y;
}Vector2;


Vector2 vector2Sub(Vector2 a, Vector2 b);
Vector2 vector2Add(Vector2 a, Vector2 b);
Vector2 vector2Scalar(Vector2 a, float b);
Vector2 vector2Normalize(Vector2 v);
float vector2Dot(Vector2 a, Vector2 b);
float vector2Cross(Vector2 a, Vector2 b);
float vector2Distance(Vector2 a, Vector2 b);
Vector2 vector2Negate(Vector2 v);
Vector2 vector2Center(Vector2 a, Vector2 b);
float vector2Angle(Vector2 a, Vector2 b);
Coord2D vectorToCoord(Vector2 v);