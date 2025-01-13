#pragma once
#include "baseTypes.h"
#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct ramp_t Ramp;

	typedef enum rampType_e
	{
		RAMP_A,
		RAMP_B,
		RAMP_C,
		RAMP_D,
		RAMP_E,
		RAMP_F,
		RAMP_G,
		RAMP_H,
		RAMP_END,

		RAMP_COUNT
	}RampType;

	void rampInitTextures();

	Ramp* rampNew(float xPosition, RampType rampType);

	void rampDelete(Ramp* ramp);

#ifdef __cplusplus
}
#endif