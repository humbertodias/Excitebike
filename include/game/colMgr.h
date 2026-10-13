#pragma once
#include "collision.h"

#ifdef __cplusplus
extern "C" {
#endif

	void colMgrInit(uint32_t maxColliders);
	void colMgrShutdown();
	void colMgrAdd(Collider* col);
	void colMgrRemove(Collider* col);

	void colMgrUpdate();


#ifdef __cplusplus
}
#endif