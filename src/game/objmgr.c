#include "platform.h"
#include <stdlib.h>
#include <assert.h>
#include "objmgr.h"
#include "baseTypes.h"

static struct objmgr_t {
    Object** list;
    uint32_t max;
    uint32_t count;
} _objMgr = { NULL, 0, 0 };

/// @brief Initialize the object manager
/// @param maxObjects 
void objMgrInit(uint32_t maxObjects)
{
    // allocate the required space
    _objMgr.list = malloc(maxObjects * sizeof(Object*));
    if (_objMgr.list != NULL) {
        // initialize as empty
        ZeroMemory(_objMgr.list, maxObjects * sizeof(Object*));
        _objMgr.max = maxObjects;
        _objMgr.count = 0;
    }

    // setup registration, so all initialized objects are logged w/ the manager
    objEnableRegistration(objMgrAdd, objMgrRemove);
}

/// @brief Shutdown the object manager
void objMgrShutdown()
{
    // disable registration, since the object manager is shutting down
    objDisableRegistration();

    // this isn't strictly required, but want to enforce proper cleanup
    assert(_objMgr.count == 0);

    // objMgr doesn't own the objects, so just clean up self
    free(_objMgr.list);
    _objMgr.list = NULL;
    _objMgr.max = _objMgr.count = 0;
}

/// @brief Add an object to be tracked by the manager
/// @param obj 
void objMgrAdd(Object* obj)
{
    for (uint32_t i = 0; i < _objMgr.max; ++i)
    {
        if (_objMgr.list[i] == NULL)
        {
            _objMgr.list[i] = obj;
            ++_objMgr.count;
            return;
        }
    }

    // out of space to add object!
    assert(false);
}

/// @brief Remove an object from the manager's tracking
/// @param obj 
void objMgrRemove(Object* obj)
{
    for (uint32_t i = 0; i < _objMgr.max; ++i)
    {
        if (obj == _objMgr.list[i])
        {
            // no need to free memory, so just clear the reference
            _objMgr.list[i] = NULL;
            --_objMgr.count;
            return;
        }
    }

    // could not find object to remove!
    assert(false);
}

/// @brief Draws all registered objects
void objMgrDraw() 
{
    for (uint32_t i = 0; i < _objMgr.max; ++i)
    {
        Object* obj = _objMgr.list[i];
        if (obj != NULL)
        {
            // TODO - consider draw order?
            objDraw(obj);
        }
    }
}

/// @brief Updates all registered objects
/// @param milliseconds 
void objMgrUpdate(uint32_t milliseconds)
{
    for (uint32_t i = 0; i < _objMgr.max; ++i)
    {
        Object* obj = _objMgr.list[i];
        if (obj != NULL)
        {
            objUpdate(obj, milliseconds);
        }
    }
}

/// <summary>
/// Gets an object at the given index if it exists
/// </summary>
/// <param name="idx">Index of object to get</param>
/// <returns>Pointer to object at index or NULL if the object does not exist</returns>
Object* objMgrGetObjAtIdx(uint32_t idx)
{
    if (_objMgr.max <= idx) return NULL;

    Object* obj = _objMgr.list[idx];
    return obj;
}

/// <summary>
/// Gets first object with specified tag if it exists
/// </summary>
/// <returns>Object pointer, NULL if object with pointer is not found</returns>
Object* objMgrGetObjByTag(const char* tag) 
{
	for(uint32_t i = 0; i < _objMgr.max; ++i)
    {
		Object* obj = _objMgr.list[i];
		assert(obj->tag != NULL);
        if (strcmp(obj->tag, tag) == 0)
        {
            return obj;
        }
    }
    return NULL;
}

/// <summary>
/// Gets all objects with specified tag if it exists
/// </summary>
/// <returns>Object pointer array</returns>
Object** objMgrGetAllObjByTag(const char* tag, uint32_t* outCount)
{
	if (tag == NULL) return NULL;

	Object** objList = malloc(sizeof(Object*) * _objMgr.count);
	uint32_t count = 0;
	for (uint32_t i = 0; i < _objMgr.max; ++i)
	{
		Object* obj = _objMgr.list[i];
		assert(obj->tag != NULL);
		if (strcmp(obj->tag, tag) == 0)
		{
			objList[count++] = obj;
		}
	}

	if (count == 0)
	{
		free(objList);
		objList = NULL;
	}
	else
	{
		objList = realloc(objList, sizeof(Object*) * count);
	}

    *outCount = count;

	return objList;
}