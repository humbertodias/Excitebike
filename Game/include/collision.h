#pragma once
#include "baseTypes.h"
#include "shape.h"
#include "vector.h"
#include "Object.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct collider_t
{
	// Parent object
	Object* obj;
	Shape* 	shape;

	// Vector2{Start, End}
	// Only visible to colliders with overlapping lane spans
	Vector2 laneSpan;

	// True if objects should consider this as a trigger,
	// False to resolve collisions
	bool	isTrigger;

	// Is true while a collision is actively occuring
	bool	isColliding;

	bool	collidingLastFrame;
}Collider;

typedef struct collision_t
{
	// The objects own collider
	Collider* self;

	// The other objects collider
	Collider* other;

	Vector2 normal;
	float	depth;
}Collision;


bool Collide(Collider* first, Collider* second, Collision* collision, uint32_t milliseconds);

#ifdef __cplusplus
}
#endif