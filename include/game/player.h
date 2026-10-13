#pragma once
#include "baseTypes.h"
#include "collision.h"
#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif


	typedef struct player_t Player;

	void playerInitTextures();
	Player* playerNew(Bounds2D box);
	void playerDelete(Player* player);
	float playerGetVelocityX();
	Collider* playerGetTrigger(Object* obj);
	void playerSetTopSpeed(Player* player, float speed);
	void playerResetTopSpeed(Player* player);

	void playerCooldown(Player* player);
	bool playerIsWheelie(Player* player);
	void playerCrash(Player* player);
	uint8_t playerGetLaneIdx(Player* player);
	void playerWin();
#ifdef __cplusplus
}
#endif