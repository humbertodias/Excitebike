#include "baseTypes.h"
#include "object.h"

#include <assert.h>
#include <string.h>

static ObjRegistrationFunc _registerFunc = NULL;
static ObjRegistrationFunc _deregisterFunc = NULL;

/// @brief Enable callback to a registrar on ObjInit/Deinit
/// @param registerFunc 
/// @param deregisterFunc 
void objEnableRegistration(ObjRegistrationFunc registerFunc, ObjRegistrationFunc deregisterFunc)
{
    _registerFunc = registerFunc;
    _deregisterFunc = deregisterFunc;
}

/// @brief Disable registration during ObjInit/Deinit
void objDisableRegistration()
{
    _registerFunc = _deregisterFunc = NULL;
}

/// @brief Initialize an object. Intended to be called from subclass constructors
/// @param obj 
/// @param vtable 
/// @param pos 
/// @param vel 
void objInit(Object* obj, ObjVtable* vtable, Coord2D pos, Coord2D vel)
{
    obj->vtable = vtable;
    obj->position = pos;
    obj->velocity = vel;
    obj->tag = "";
    if (_registerFunc != NULL)
    {
        _registerFunc(obj);
    }
}

/// @brief Deinitialize an object
/// @param obj 
void objDeinit(Object* obj)
{
    if (_deregisterFunc != NULL)
    {
        _deregisterFunc(obj);
    }
}

/// @brief Draw this object, using it's vtable
/// @param obj 
void objDraw(Object* obj)
{
    if (obj->vtable != NULL && obj->vtable->draw != NULL) 
    {
        obj->vtable->draw(obj);
    }
}

/// @brief Update this object, using it's vtable
/// @param obj 
/// @param milliseconds 
void objUpdate(Object* obj, uint32_t milliseconds)
{
    if (obj->vtable != NULL && obj->vtable->update != NULL) 
    {
        obj->vtable->update(obj, milliseconds);
        return;
    }

    objDefaultUpdate(obj, milliseconds);
}

void objCollision(Object* obj, Collision* collision, uint32_t milliseconds)
{
	if( obj->vtable != NULL && obj->vtable->onCollision != NULL)
	{
        obj->vtable->onCollision(collision, milliseconds);
	}
}

void objTrigger(Object* obj, Collision* collision, uint32_t milliseconds)
{
	if (obj->vtable != NULL && obj->vtable->onTrigger != NULL)
	{
		obj->vtable->onTrigger(collision, milliseconds);
	}
}

void objCollisionEnter(Object* obj, Collision* collision, uint32_t milliseconds)
{
	if (obj->vtable != NULL && obj->vtable->onCollisionEnter != NULL)
	{
		obj->vtable->onCollisionEnter(collision, milliseconds);
	}
}

void objTriggerEnter(Object* obj, Collision* collision, uint32_t milliseconds)
{
	if (obj->vtable != NULL && obj->vtable->onTriggerEnter != NULL)
	{
		obj->vtable->onTriggerEnter(collision, milliseconds);
	}
}

void objCollisionExit(Object* obj, Collision* collision, uint32_t milliseconds)
{
	if (obj->vtable != NULL && obj->vtable->onCollisionExit != NULL)
	{
		obj->vtable->onCollisionExit(collision, milliseconds);
	}
}

void objTriggerExit(Object* obj, Collision* collision, uint32_t milliseconds)
{
	if (obj->vtable != NULL && obj->vtable->onTriggerExit != NULL)
	{
		obj->vtable->onTriggerExit(collision, milliseconds);
	}
}

Collider* objGetCollider(Object* obj)
{
    Collider* collider = NULL;
    if( obj->vtable != NULL && obj->vtable->getCollider != NULL)
    {
        collider = obj->vtable->getCollider(obj);
    }
    return collider;
}

void objDefaultUpdate(Object* obj, uint32_t milliseconds)
{
    obj->position.x += obj->velocity.x;
    obj->position.y += obj->velocity.y;
}

bool objCompareTag(Object* obj, const char* tag)
{
    if (strcmp(obj->tag, tag) == 0) return true;
    return false;
}