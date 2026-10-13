#pragma once
#include "baseTypes.h"
//#include "collision.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct collider_t Collider;
typedef struct collision_t Collision;

// object "virtual" functions
typedef struct object_t Object;
typedef void (*ObjDrawFunc)(Object*);
typedef void (*ObjUpdateFunc)(Object*, uint32_t);
typedef void (*ObjCollisionFunc)(Collision*, uint32_t);
typedef void (*ObjColTriggerFunc)(Collision*, uint32_t);
typedef void (*ObjColEnterFunc)(Collision*, uint32_t);
typedef void (*ObjTrigEnterFunc)(Collision*, uint32_t);
typedef void (*ObjColExitFunc)(Collision*, uint32_t);
typedef void (*ObjTrigExitFunc)(Collision*, uint32_t);
typedef Collider* (*ObjGetColFunc)(Object*);

typedef struct object_vtable_t {
    ObjDrawFunc     	draw;
    ObjUpdateFunc   	update;
	ObjCollisionFunc  	onCollision;
    ObjColTriggerFunc 	onTrigger;
    ObjColEnterFunc     onCollisionEnter;
    ObjTrigEnterFunc    onTriggerEnter;
    ObjColExitFunc      onCollisionExit;
    ObjTrigExitFunc     onTriggerExit;
    ObjGetColFunc 		getCollider;
} ObjVtable;

typedef struct object_t {
    ObjVtable*      vtable;
    Coord2D         position;
    Coord2D         velocity;
    const char* 	tag;
} Object;

typedef void (*ObjRegistrationFunc)(Object*);

// class-wide registration methods
void objEnableRegistration(ObjRegistrationFunc registerFunc, ObjRegistrationFunc deregisterFunc);
void objDisableRegistration();

// object API
void objInit(Object* obj, ObjVtable* vtable, Coord2D pos, Coord2D vel);
void objDeinit(Object* obj);
void objDraw(Object* obj);
void objUpdate(Object* obj, uint32_t milliseconds);
void objCollision(Object* obj, Collision* collision, uint32_t milliseconds);
void objTrigger(Object* obj, Collision* collision, uint32_t milliseconds);
void objCollisionEnter(Object* obj, Collision* collision, uint32_t milliseconds);
void objTriggerEnter(Object* obj, Collision* collision, uint32_t milliseconds);
void objCollisionExit(Object* obj, Collision* collision, uint32_t milliseconds);
void objTriggerExit(Object* obj, Collision* collision, uint32_t milliseconds);

Collider* objGetCollider(Object* obj);
// default update implementation that just moves at the current velocity
void objDefaultUpdate(Object* obj, uint32_t milliseconds);

bool objCompareTag(Object* obj, const char* tag);
#ifdef __cplusplus
}
#endif