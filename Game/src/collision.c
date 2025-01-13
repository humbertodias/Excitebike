#include "collision.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "shape.h"

typedef bool(*CollisionFunction)(const Shape* first, const Shape* second, Vector2 *normal, float* depth);
bool collisionCirclePolygon(const Shape* circle, const Shape* polygon, Vector2* normal, float* depth);
bool collisionPolygonCircle(const Shape* polygon, const Shape* circle, Vector2* normal, float* depth);

CollisionFunction CollisionTable[_SHAPE_COUNT][_SHAPE_COUNT] = 
{
	/*              Point, Circle, Rect, Trapezoid */
	/*     Point */{NULL, NULL, collisionCirclePolygon, NULL},
	/*    Circle */{NULL, NULL, NULL, collisionCirclePolygon},
	/*      Rect */{NULL, collisionPolygonCircle, NULL, NULL},
	/* Trapezoid */{NULL, collisionPolygonCircle, NULL, NULL}
};

Vector2 findArithmeticMean(Vector2 *vertices, int vertexCount)
{
	assert(vertices != NULL);
	float sumX = 0.0f;
	float sumY = 0.0f;

	for (int i = 0; i < vertexCount; i++)
	{
		Vector2 v = vertices[i];
		sumX += v.x;
		sumY += v.y;
	}

	return (Vector2){sumX / (float)vertexCount, sumY / (float)vertexCount};
}

int findClosestPointOnPolygon(Vector2 circleCenter, Vector2* vertices, uint32_t vertexCount)
{
	int result = -1;
	float minDistance = FLT_MAX;

	for (uint32_t i = 0; i < vertexCount; i++)
	{
		Vector2 v = vertices[i];
		float distance = vector2Distance(v, circleCenter);

		if (distance < minDistance)
		{
			minDistance = distance;
			result = i;
		}
	}

	return result;
}

void projectCircle(Vector2 center, float radius, Vector2 axis, float *min, float *max)
{
	Vector2 direction = vector2Normalize(axis);
	Vector2 directionAndRadius = vector2Scalar(direction, radius);

	Vector2 p1 = vector2Add(center, directionAndRadius);
	Vector2 p2 = vector2Sub(center, directionAndRadius);

	*min = vector2Dot(p1, axis);
	*max = vector2Dot(p2, axis);

	if (*min > *max)
	{
		// swap the min and max values.
		float t = *min;
		*min = *max;
		*max = t;
	}
}

void projectVertices(Vector2* vertices, uint32_t vertexCount, Vector2 axis, float *min, float *max)
{
	*min = FLT_MAX;
	*max = FLT_MIN;

	for (uint32_t i = 0; i < vertexCount; i++)
	{
		Vector2 v = vertices[i];
		float proj = vector2Dot(v, axis);

		if (proj < *min)
		{
			*min = proj;
		}

		if (proj > *max)
		{
			*max = proj;
		}
	}
}


/// @brief Returns true if an intersection occurs, Credit to https://www.youtube.com/watch?v=vWs33LVrs74
/// @param circle
/// @param polygon
/// @param normal Out intersect normal
/// @param depth Out intersect depth
///	@return True: On collision
///	@returns False: On no collision
bool collisionCirclePolygon(const Shape* circle, const Shape* polygon, Vector2* normal, float* depth)
{
	ShapeCircle* _circle = circle->circle;
	ShapeTrapezoid* _trapezoid = polygon->trapezoid;

	uint8_t numPoints = 6;
	Vector2 vertices[6];

	if (polygon->type == SHAPE_RECTANGLE)
	{
		float halfX = polygon->rectangle->size.x / 2;
		float halfY = polygon->rectangle->size.y / 2;
		Vector2 topLeft = { polygon->rectangle->position.x - halfX, polygon->rectangle->position.y - halfY };
		Vector2 botRight = { polygon->rectangle->position.x + halfX, polygon->rectangle->position.y + halfY };
		vertices[0] = topLeft;
		vertices[1] = (Vector2){ botRight.x, topLeft.y };
		vertices[2] = botRight;
		vertices[3] = (Vector2){ topLeft.x, botRight.y };
		numPoints = 4;
	}
	else
	{
		vertices[0] = _trapezoid->a;
		vertices[1] = _trapezoid->b;
		vertices[2] = _trapezoid->c;
		vertices[3] = _trapezoid->d;
		vertices[4] = (Vector2){ _trapezoid->d.x, _trapezoid->d.y + 300 };
		vertices[5] = (Vector2){ _trapezoid->a.x, _trapezoid->a.y + 300 };
	};
	
	//public static bool IntersectCirclePolygon(Vector2 circleCenter, float circleRadius, 
	//Vector2[] vertices,
	//	out Vector2 normal, out float depth)


	*normal = (Vector2){ 0, 0 };
	*depth = FLT_MAX;

	Vector2 axis = { 0, 0 };
	float axisDepth = 0.0f;
	float minA, maxA, minB, maxB;


	for (int i = 0; i < numPoints; i++)
	{
		Vector2 va = vertices[i];
		Vector2 vb = vertices[(i + 1) % numPoints];

		Vector2 edge = vector2Sub(vb, va);
		axis = (Vector2){-edge.y, edge.x};
		axis = vector2Normalize(axis);

		projectVertices(vertices, numPoints, axis, &minA, &maxA);
		projectCircle(_circle->center, _circle->radius, axis, &minB, &maxB);

		if (minA >= maxB || minB >= maxA)
		{
			return false;
		}

		axisDepth = (float)fmin(maxB - minA, maxA - minB);

		if (axisDepth < *depth)
		{
			*depth = axisDepth;
			*normal = axis;
		}
	}

	int cpIndex = findClosestPointOnPolygon(_circle->center, vertices, numPoints);
	Vector2 cp = vertices[cpIndex];

	axis = vector2Sub(cp, _circle->center);
	axis = vector2Normalize(axis);

	projectVertices(vertices, numPoints, axis, &minA, &maxA);
	projectCircle(_circle->center, _circle->radius, axis, &minB, &maxB);

	if (minA >= maxB || minB >= maxA)
	{
		return false;
	}

	axisDepth = (float)fmin(maxB - minA, maxA - minB);

	if (axisDepth < *depth)
	{
		*depth = axisDepth;
		*normal = axis;
	}

	Vector2 polygonCenter = findArithmeticMean(vertices, numPoints);

	Vector2 direction = vector2Sub(polygonCenter, _circle->center);

	if (vector2Dot(direction, *normal) < 0.0f)
	{
		*normal = vector2Negate(*normal);
	}

	return true;
}

bool collisionPolygonCircle(const Shape* polygon, const Shape* circle, Vector2* normal, float* depth)
{
	return collisionCirclePolygon(circle, polygon, normal, depth);
}

/// <summary>
/// Checks for collisions between two colliders and
///	will call both parent objects onCollision or onTrigger functions if there is a collision
/// </summary>
/// <param name="first">Collider of the first object</param>
/// <param name="second">Collider of the second object</param>
/// <param name="collision"></param>
/// <param name="milliseconds"></param>
/// <returns>True when a collision occurs, False on no collision</returns>
bool Collide(Collider* first, Collider* second, Collision* collision, uint32_t milliseconds)
{
	assert(("First collider is null", first != NULL));
	assert(("Second collider is null", second != NULL));

	static bool	hadCollision = false;
	static bool collisionWasTrigger = false;
	static Collision lastCollision = { NULL, NULL, {0, 0}, 0 };

	CollisionFunction func = CollisionTable[first->shape->type][second->shape->type];

	if(func != NULL)
	{
		assert(first->obj != NULL);
		assert(second->obj != NULL);
		Vector2 normal = {0, 0};
		float depth = 0;
		if (func(first->shape, second->shape, &normal, &depth))
		{
			Collision colObjFirst;
			Collision colObjSecond;

			colObjFirst.normal = normal;
			colObjFirst.depth = depth;
			colObjFirst.self = first;
			colObjFirst.other = second;

			colObjSecond.normal = normal;
			colObjSecond.depth = depth;
			colObjSecond.self = second;
			colObjSecond.other = first;


			first->collidingLastFrame = first->isColliding;
			second->collidingLastFrame = second->isColliding;
			first->isColliding = true;
			second->isColliding = true;

			if( first->isTrigger || second->isTrigger)
			{

				if (first->isColliding && !first->collidingLastFrame)
				{
					objTriggerEnter(first->obj, &colObjFirst, milliseconds);
				}

				if (second->isColliding && !second->collidingLastFrame)
				{
					objTriggerEnter(second->obj, &colObjSecond, milliseconds);
				}

				objTrigger(first->obj, &colObjFirst, milliseconds);
				objTrigger(second->obj, &colObjSecond, milliseconds);
				collisionWasTrigger = true;
			}
			else
			{
				if (first->isColliding && !first->collidingLastFrame)
				{
					objCollisionEnter(first->obj, &colObjFirst, milliseconds);
				}
				if (second->isColliding && !second->collidingLastFrame)
				{
					objCollisionEnter(second->obj, &colObjSecond, milliseconds);
				}

				objCollision(first->obj, &colObjFirst, milliseconds);
				objCollision(second->obj, &colObjSecond, milliseconds);
				collisionWasTrigger = false;
			}

			hadCollision = true;
			lastCollision = colObjFirst;
			// TODO: free(collision); 
			return true;
		}

		if(hadCollision)
		{
			if( first == lastCollision.self && second == lastCollision.other)
			{
				if( collisionWasTrigger )
				{
					objTriggerExit(first->obj, &lastCollision, milliseconds);
					lastCollision.self->obj = second->obj;
					lastCollision.other->obj = first->obj;
					objTriggerExit(second->obj, &lastCollision, milliseconds);
				}
				else
				{
					objCollisionExit(first->obj, &lastCollision, milliseconds);
					lastCollision.self->obj = second->obj;
					lastCollision.other->obj = first->obj;
					objCollisionExit(second->obj, &lastCollision, milliseconds);
				}
				lastCollision.self->obj = first->obj;
				lastCollision.other->obj = second->obj;
				hadCollision = false;
			}
		}
		first->collidingLastFrame = first->isColliding;
		second->collidingLastFrame = second->isColliding;
		first->isColliding = false;
		second->isColliding = false;

	}
	return false;
}