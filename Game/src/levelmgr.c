#include <Windows.h>
#include <stdlib.h>
#include "baseTypes.h"
#include "levelmgr.h"

#include "background.h"
#include "menu.h"
#include "player.h"
#include "ramp.h"
#include "sound.h"
#include "surface.h"

typedef struct level_t
{
    const LevelDef* def;

	Menu* menu;
    Background* background;
    Player* player;
    Surface** surfaces;
    Ramp** ramps;
} Level;

static int32_t _soundId = SOUND_NOSOUND;

/// @brief Initialize the level manager
void levelMgrInit()
{
	surfaceInitTextures();
    rampInitTextures();
    backgroundInitTextures();
	menuInitTextures();
	menuInitSounds();


}

/// @brief Shutdown the level manager
void levelMgrShutdown()
{
}

/// @brief Loads the level and all required objects/assets
/// @param levelDef 
/// @return 
Level* levelMgrLoad(const LevelDef* levelDef)
{
    Level* level = malloc(sizeof(Level));
    if (level != NULL)
    {
		level->def = levelDef;
		if( level->def->hasMenu)
		{
			level->menu = menuNew(level->def->menuIdx);
		}
		else
		{
			level->background = backgroundNew();
		}

		level->surfaces = malloc(levelDef->numSurfaces * sizeof(Surface*));
		level->ramps = malloc(levelDef->numRamps * sizeof(Ramp*));
		//if (level->surfaces != NULL)
		if( level->def->numSurfaces == 15) // The right way to do this is not like this, I know that, this is purely to meet the deadline
		{
			level->surfaces[0] = surfaceNew(4.5f * 976, SURFACE_DIRT, 1);
			level->surfaces[1] = surfaceNew(4.5f * 976, SURFACE_DIRT, 3);
			level->surfaces[2] = surfaceNew(4.5f * 1720, SURFACE_COOLDOWN, 3);
			level->surfaces[3] = surfaceNew(4.5f * 2648, SURFACE_COOLDOWN, 0);
			level->surfaces[4] = surfaceNew(4.5f * 2792, SURFACE_DIRT, 0);
			level->surfaces[5] = surfaceNew(4.5f * 2792, SURFACE_DIRT, 2);
			level->surfaces[6] = surfaceNew(4.5f * 3680, SURFACE_COOLDOWN, 0);
			level->surfaces[7] = surfaceNew(4.5f * 4480, SURFACE_GRASS_DOUBLE, 0);
			level->surfaces[8] = surfaceNew(4.5f * 5272, SURFACE_DIRT, 0);
			level->surfaces[9] = surfaceNew(4.5f * 5272, SURFACE_DIRT, 2);
			level->surfaces[10] = surfaceNew(4.5f * 5384, SURFACE_DIRT, 1);
			level->surfaces[11] = surfaceNew(4.5f * 5384, SURFACE_DIRT, 3);
			level->surfaces[12] = surfaceNew(4.5f * 5520, SURFACE_DIRT, 0);
			level->surfaces[13] = surfaceNew(4.5f * 5520, SURFACE_DIRT, 2);
			level->surfaces[14] = surfaceNew(4.5f * 5756, SURFACE_CHECKER_WIN, 4);

			level->ramps[0] = rampNew(4.5f * 1056, RAMP_C);
			level->ramps[1] = rampNew(4.5f * 1384, RAMP_B);
			level->ramps[2] = rampNew(4.5f * 1816, RAMP_B);
			level->ramps[3] = rampNew(4.5f * 1872, RAMP_B);
			level->ramps[4] = rampNew(4.5f * 2384, RAMP_B);
			level->ramps[5] = rampNew(4.5f * 2440, RAMP_B);
			level->ramps[6] = rampNew(4.5f * 2496, RAMP_B);
			level->ramps[7] = rampNew(4.5f * 2944, RAMP_F);
			level->ramps[8] = rampNew(4.5f * 3036, RAMP_G);
			level->ramps[9] = rampNew(4.5f * 3424, RAMP_H);
			level->ramps[10] = rampNew(4.5f * 3712, RAMP_H);
			level->ramps[11] = rampNew(4.5f * 4032, RAMP_H);
			level->ramps[12] = rampNew(4.5f * 4064, RAMP_A);
			level->ramps[13] = rampNew(4.5f * 4104, RAMP_A);
			level->ramps[14] = rampNew(4.5f * 4144, RAMP_A);
			level->ramps[15] = rampNew(4.5f * 4184, RAMP_A);
			level->ramps[16] = rampNew(4.5f * 4368, RAMP_F);
			level->ramps[17] = rampNew(4.5f * 4688, RAMP_D);
			level->ramps[18] = rampNew(4.5f * 4776, RAMP_D);
			level->ramps[19] = rampNew(4.5f * 4896, RAMP_D);
			level->ramps[20] = rampNew(4.5f * 4984, RAMP_D);
			level->ramps[21] = rampNew(4.5f * 5656, RAMP_END);
		}

		if (level->def->numSurfaces == 21)
		{
			level->surfaces[0] = surfaceNew(4.5f * 952, SURFACE_BUMP, 0);
			level->surfaces[1] = surfaceNew(4.5f * 1016, SURFACE_BUMP, 2);
			level->surfaces[2] = surfaceNew(4.5f * 1120, SURFACE_BUMP, 0);
			level->surfaces[3] = surfaceNew(4.5f * 1208, SURFACE_BUMP, 2);
			level->surfaces[4] = surfaceNew(4.5f * 1328, SURFACE_BUMP, 0);
			level->surfaces[5] = surfaceNew(4.5f * 1456, SURFACE_BUMP, 2);
			level->surfaces[6] = surfaceNew(4.5f * 2080, SURFACE_COOLDOWN, 0);
			level->surfaces[7] = surfaceNew(4.5f * 2112, SURFACE_COOLDOWN, 3);
			level->surfaces[8] = surfaceNew(4.5f * 2566, SURFACE_GRASS_SINGLE, 0);
			level->surfaces[9] = surfaceNew(4.5f * 3443, SURFACE_GRASS_DOUBLE, 0);
			level->surfaces[10] = surfaceNew(4.5f * 4016, SURFACE_BUMP, 0);
			level->surfaces[11] = surfaceNew(4.5f * 4632, SURFACE_COOLDOWN, 0);
			level->surfaces[12] = surfaceNew(4.5f * 4758, SURFACE_GRASS_SINGLE, 0);
			level->surfaces[13] = surfaceNew(4.5f * 5318, SURFACE_GRASS_SINGLE, 2);
			level->surfaces[14] = surfaceNew(4.5f * 5784, SURFACE_BUMP, 2);
			level->surfaces[15] = surfaceNew(4.5f * 5856, SURFACE_DIRT, 1);
			level->surfaces[16] = surfaceNew(4.5f * 5856, SURFACE_DIRT, 3);
			level->surfaces[17] = surfaceNew(4.5f * 5920, SURFACE_DIRT, 0);
			level->surfaces[18] = surfaceNew(4.5f * 5920, SURFACE_DIRT, 2);
			level->surfaces[19] = surfaceNew(4.5f * 5984, SURFACE_BUMP, 0);
			level->surfaces[20] = surfaceNew(4.5f * 6268, SURFACE_CHECKER_WIN, 4);

			level->ramps[0] = rampNew(4.5f * 1592, RAMP_B);
			level->ramps[1] = rampNew(4.5f * 1664, RAMP_B);
			level->ramps[2] = rampNew(4.5f * 1857, RAMP_F);
			level->ramps[3] = rampNew(4.5f * 1980, RAMP_G);
			level->ramps[4] = rampNew(4.5f * 2185, RAMP_F);
			level->ramps[5] = rampNew(4.5f * 2249, RAMP_E);
			level->ramps[6] = rampNew(4.5f * 2308, RAMP_G);
			level->ramps[7] = rampNew(4.5f * 3272, RAMP_H);
			level->ramps[8] = rampNew(4.5f * 3360, RAMP_B);
			level->ramps[9]= rampNew(4.5f * 3728, RAMP_H);
			level->ramps[10] = rampNew(4.5f * 3822, RAMP_C);
			level->ramps[11] = rampNew(4.5f * 4136, RAMP_C);
			level->ramps[12] = rampNew(4.5f * 4260, RAMP_G);
			level->ramps[13] = rampNew(4.5f * 4352, RAMP_F);
			level->ramps[14] = rampNew(4.5f * 4440, RAMP_C);
			level->ramps[15] = rampNew(4.5f * 6176, RAMP_END);
		}

		//level->player = malloc(sizeof(Player*));
		if( level->def->hasPlayer)
		{
			level->player = playerNew((Bounds2D) { { 0, 0 }, { 100, 100 } });
		}
    }
    return level;
}

/// @brief Unloads the level and frees up any assets associated
/// @param level 
void levelMgrUnload(Level* level)
{
    if (level != NULL) 
    {
		if( level->def->hasMenu)
		{
			menuDelete(level->menu);
		}
    	else
		{
			backgroundDelete(level->background);
			playerDelete(level->player);
		}

        for (uint32_t i = 0; i < level->def->numSurfaces; ++i)
        {
            surfaceDelete(level->surfaces[i]);
        }

        free(level->surfaces);

        for( uint32_t i = 0; i < level->def->numRamps; ++i)
        {
            rampDelete(level->ramps[i]);
        }
        free(level->ramps);

    }
    free(level);
}