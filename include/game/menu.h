#pragma once
#include "baseTypes.h"
#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct menu_t Menu;

	typedef enum menuType_e
	{
		MENU_MAIN,
		MENU_TRACKS,
		MENU_END,

		MENU_COUNT
	}MenuType;

	void menuInitTextures();

	void menuInitSounds();
	Menu* menuNew(MenuType menuType);

	void menuDelete(Menu* menu);

#ifdef __cplusplus
}
#endif