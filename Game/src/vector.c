
#include "vector.h"

#include <corecrt_math_defines.h>
#include <math.h>

Vector2 vector2Sub(Vector2 a, Vector2 b)
{
	return (Vector2){ a.x - b.x, a.y - b.y };
}

Vector2 vector2Add(Vector2 a, Vector2 b)
{
	return (Vector2){ a.x + b.x, a.y + b.y };
}

Vector2 vector2Scalar(Vector2 a, float b)
{
	return (Vector2){ a.x * b, a.y * b };
}

Vector2 vector2Normalize(Vector2 v)
{
	float magnitude = 1.0f / (float)sqrt(v.x * v.x + v.y * v.y);
	return (Vector2){ v.x * magnitude, v.y * magnitude };
}

float vector2Dot(Vector2 a, Vector2 b)
{
	return a.x * b.x + a.y * b.y;
}

float vector2Cross(Vector2 a, Vector2 b)
{
	return a.x * b.y - a.y * b.x;
}

float vector2Distance(Vector2 a, Vector2 b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return (float)sqrt(dx * dx + dy * dy);
}

Vector2 vector2Negate(Vector2 v)
{
	return (Vector2){ -v.x, -v.y };
}

Vector2 vector2Center(Vector2 a, Vector2 b) {
	Vector2 center;
	center.x = (a.x + b.x) / 2.0f;
	center.y = (a.y + b.y) / 2.0f;
	return center;
}

float vector2Angle(Vector2 a, Vector2 b) {
	float dx = b.x - a.x;
	float dy = -b.y + a.y;

	float angle = (float)atan2(dy, dx); 

	if (angle < 0) {
		angle += 2 * (float)M_PI;
	}

	return angle;
}

Coord2D vectorToCoord(Vector2 v)
{
	return (Coord2D){ v.x, v.y };
}