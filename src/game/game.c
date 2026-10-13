#include <assert.h>
#include "platform.h"
#include "game.h"
#include "baseTypes.h"
#include "input.h"
#include "application.h"
#include "framework.h"

#include "levelmgr.h"
#include "objmgr.h"

#ifdef USE_SDL
#include <SDL.h>
#endif

static void _gameInit();
static void _gameShutdown();
static void _gameDraw();
static void _gameUpdate(uint32_t milliseconds);
void _gameLoadTrackSelect();

static bool loadTrackSelect = false;

static LevelDef _levelDefs[] = {
	{
		0,							// numSurfaces
		0,
		false,
		true,
		0
	},
	{
		0,							// numSurfaces
		0,
		false,
		true,
		1
	},
	{
		15,
		22,
		true,
		false,
		1
	},
	{
		21,
		16,
		true,
		false,
		1
	}
};
static Level* _curLevel = NULL;

/// @brief Program Entry Point (WinMain)
/// @param hInstance 
/// @param hPrevInstance 
/// @param lpCmdLine 
/// @param nCmdShow 
/// @return 
#if defined(_WIN32) && !defined(USE_SDL)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char** argv)
#endif
{
	const char GAME_NAME[] = "Excitebike";
#if defined(_WIN32) && !defined(USE_SDL)
	HINSTANCE instance = hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nCmdShow;
#else
	HINSTANCE instance = NULL;
	(void)argc;
	(void)argv;
#endif

	Application* app = appNew(instance, GAME_NAME, _gameDraw, _gameUpdate);
	appSetWidth(app, 1143);
	appSetHeight(app, 1080);
	if (app != NULL)
	{
		GLWindow* window = fwInitWindow(app);
		if (window != NULL)
		{
			_gameInit();

			bool running = true;
			while (running)
			{
				running = fwUpdateWindow(window);
			}

			_gameShutdown();
			fwShutdownWindow(window);
		}

		appDelete(app);
	}

	return 0;
}

/// @brief Initialize code to run at application startup
static void _gameInit()
{
	const uint32_t MAX_OBJECTS = 500;
	objMgrInit(MAX_OBJECTS);
	levelMgrInit();

	_curLevel = levelMgrLoad(&_levelDefs[0]);

	ShowCursor(true);
}

/// @brief Cleanup the game and free up any allocated resources
static void _gameShutdown()
{
	levelMgrUnload(_curLevel);

	levelMgrShutdown();
	objMgrShutdown();
}

/// @brief Draw everything to the screen for current frame
static void _gameDraw() 
{
	objMgrDraw();
	if(loadTrackSelect)
	{
		_gameLoadTrackSelect();
	}
}

/// @brief Perform updates for all game objects, for the elapsed duration
/// @param milliseconds 
static void _gameUpdate(uint32_t milliseconds)
{
	// ESC exits the program
	if (inputKeyPressed(VK_ESCAPE))
	{
		// TODO 
		//TerminateApplication(window);
	}

	// F1 toggles fullscreen
	if (inputKeyPressed(VK_F1))
	{
		// TODO 
		//ToggleFullscreen(window);
	}

	objMgrUpdate(milliseconds);
}

void gameLoadTrack(uint8_t trackNumber)
{
	levelMgrUnload(_curLevel);
	_curLevel = levelMgrLoad(&_levelDefs[trackNumber + 1]);
}

void gameLoadTrackSelect()
{
	loadTrackSelect = true;
}

void gameQuit()
{
	_gameShutdown();
}

void _gameLoadTrackSelect()
{
	loadTrackSelect = false;
	levelMgrUnload(_curLevel);
	_curLevel = levelMgrLoad(&_levelDefs[1]);
}