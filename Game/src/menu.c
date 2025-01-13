#include "menu.h"

#include <assert.h>
#include <stdlib.h>
#include <Windows.h>
#include <GL/gl.h>

#include "game.h"
#include "input.h"
#include "object.h"
#include "SOIL.h"
#include "sound.h"
#include "sprite.h"
//
typedef struct menu_t
{
	Object 		obj;
	MenuType 	type;
	struct
	{
		Vector2 position;
		uint8_t idx;
		uint8_t min;
		uint8_t max;
	}selector;

}Menu;


static GLuint menuSprites[4];
static int32_t menuSounds[3];

static const char MAIN_PAGE[] = "asset/menu/main.png";
static const char TRACKS_PAGE[] = "asset/menu/tracks.png";
static const char END_PAGE[] = "asset/menu/end.png";

static const char SELECTOR[] = "asset/menu/selector.png";

static const Vector2 options[] =
{
	{159, 897},
	{290, 800},
	{430, 800},
};



static void _menuDraw(Object* obj);
static void _menuUpdate(Object* obj, uint32_t milliseconds);

void _activateMenu(Menu* menu, MenuType type);



static ObjVtable _menuVtable = {
	_menuDraw,
	_menuUpdate
};

void menuInitTextures()
{
	menuSprites[0] = SOIL_load_OGL_texture(MAIN_PAGE, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	assert(menuSprites[0] != 0);

	glBindTexture(GL_TEXTURE_2D, menuSprites[0]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	menuSprites[1] = SOIL_load_OGL_texture(TRACKS_PAGE, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	assert(menuSprites[0] != 0);

	glBindTexture(GL_TEXTURE_2D, menuSprites[1]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	menuSprites[3] = SOIL_load_OGL_texture(SELECTOR, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	assert(menuSprites[0] != 0);

	glBindTexture(GL_TEXTURE_2D, menuSprites[3]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	
}

void menuInitSounds()
{
	menuSounds[0] = soundLoad("asset/menu/sound/main.wav");
	menuSounds[1] = soundLoad("asset/menu/sound/tracks.wav");
}

Menu* menuNew(MenuType menuType)
{
	Menu* menu = malloc(sizeof(Menu));
	if( menu!= NULL)
	{
		objInit(&menu->obj, &_menuVtable, (Coord2D) { 0, 0 }, (Coord2D){0, 0}); // TODO change coord2d to vector2

	}
	_activateMenu(menu, menuType);
	return menu;
}

void _menuDraw(Object* obj)
{
	Menu* menu = (Menu*)obj;
	spriteDrawFromSheet(menuSprites[menu->type], (Vector2){0,0}, (Vector2){1143, 1080}, (Vector2){0, 0},(Vector2){1143, 1080}, 1, 0.9f, false);
	spriteDrawFromSheet(menuSprites[3], menu->selector.position, (Vector2) { 40, 46 }, (Vector2) { 0, 0 }, (Vector2) { 40, 46 }, 1, 1.0f, false);

}

void _menuUpdate(Object* obj, uint32_t milliseconds)
{
	const int32_t inputCooldown = 100;
	static int32_t nextInput = 0;
	Menu* menu = (Menu*)obj;

	if( nextInput > 0)
	{
		nextInput -= milliseconds;
	}
	else
	{

		if (inputKeyPressed(VK_RETURN))
		{
			if (menu->type == MENU_MAIN)
				_activateMenu((Menu*)obj, MENU_TRACKS);

			else if (menu->type == MENU_TRACKS)
			{
				gameLoadTrack(menu->selector.idx);
			}
		}

		if(inputKeyPressed(VK_LEFT))
		{
			if (menu->selector.idx == menu->selector.min)
				menu->selector.idx = menu->selector.max;
			else
				--menu->selector.idx;
		}

		if (inputKeyPressed(VK_RIGHT))
		{
			if (menu->selector.idx == menu->selector.max)
				menu->selector.idx = menu->selector.min;
			else
				++menu->selector.idx;
		}

		menu->selector.position = options[menu->selector.idx];

		nextInput = inputCooldown;
	}
}

void menuDelete(Menu* menu)
{
	objDeinit(&menu->obj);

	free(menu);
}

void _activateMenu(Menu* menu, MenuType type)
{
	if (menu->type == type) return;

	if (menu->type == MENU_MAIN)
		soundStop(menuSounds[MENU_MAIN]);

	menu->type = type;

	switch(type)
	{
	case MENU_MAIN:
		menu->selector.min = 0;
		menu->selector.max = 0;
		break;
	case MENU_TRACKS:
		menu->selector.min = 1;
		menu->selector.max = 2;
	}

	menu->selector.idx = menu->selector.min;
	soundPlay(menuSounds[type]);
}