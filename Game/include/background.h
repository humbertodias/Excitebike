#pragma once
#include "baseTypes.h"
#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif


	typedef struct background_t Background;

	void backgroundInitTextures();
	Background* backgroundNew();
	void backgroundDelete(Background* background);
	void backgroundWin();
#ifdef __cplusplus
}
#endif