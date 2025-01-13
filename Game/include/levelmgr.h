#pragma once
#include "baseTypes.h"
#include "object.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct leveldef_t {
    uint32_t numSurfaces;
    uint32_t numRamps;
    bool    hasPlayer;
    bool    hasMenu;
    uint8_t menuIdx;
} LevelDef;

typedef struct level_t Level;

void levelMgrInit();
void levelMgrShutdown();
Level *levelMgrLoad(const LevelDef* levelDef);
void levelMgrUnload(Level* level);

#ifdef __cplusplus
}
#endif