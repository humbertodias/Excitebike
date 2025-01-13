#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <stdint.h>
#include <windows.h>
#include <GL/gl.h>

#include "Object.h"
#include "vector.h"

typedef enum shapeType_e
{
	SHAPE_POINT,
	SHAPE_CIRCLE,
	SHAPE_RECTANGLE,
	SHAPE_TRAPEZOID,

	_SHAPE_COUNT
}ShapeType;

typedef struct shapePoint_t
{
	Vector2 position;
}ShapePoint;

typedef struct shapeCircle_t
{
	Vector2 center;
	float radius;
}ShapeCircle;

typedef struct shapeRectangle_t
{
	Vector2 position;
	Vector2 size;
}ShapeRectangle;

/* Trapezoid shape defined by
*      b------c
*     /	       \
*    a----------d
*/
typedef struct shapeTrapezoid_t //0x0073F6F4
{
	Vector2 a;
	Vector2 b;
	Vector2 c;
	Vector2 d;
}ShapeTrapezoid;

typedef struct shape_t
{
	union
	{
		ShapePoint*		point;
		ShapeCircle*	circle;
		ShapeRectangle* rectangle;
		ShapeTrapezoid*	trapezoid;
	};
	ShapeType type;
	Vector2 velocity;
}Shape;

void shapeDrawCircle(float radius, float x, float y, uint8_t r, uint8_t g, uint8_t b, bool filled);
void shapeDrawLine(float startX, float startY, float endX, float endY, uint8_t r, uint8_t g, uint8_t b, GLfloat width);


void shapeDrawTrapezoid(ShapeTrapezoid* shape, uint8_t r, uint8_t g, uint8_t b);
void shapeDrawRect(Vector2 topLeft, Vector2 botRight, uint8_t r, uint8_t g, uint8_t b, GLfloat width,bool filled);
void shapeMove(Shape* shape, Vector2 transform);

void shapeSetPosition(Shape* shape, Vector2 position);

#ifdef __cplusplus
}
#endif