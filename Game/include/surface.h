#pragma once
#include "baseTypes.h"
#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct surface_t Surface;

	typedef enum surfaceType_e
	{
		SURFACE_BUMP,
		SURFACE_COOLDOWN,
		SURFACE_DIRT,
		SURFACE_GRASS_SINGLE,
		SURFACE_GRASS_DOUBLE,
		SURFACE_CHECKER_WIN,

		SURFACE_COUNT
	}SurfaceType;

	void surfaceInitTextures();

	Surface* surfaceNew(float xPosition, SurfaceType surfaceType, uint8_t laneStart);

	void surfaceDelete(Surface* surface);

#ifdef __cplusplus
}
#endif