#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "framework.h"
#include "gl_compat.h"
#include "input.h"
#include "openglDraw.h"
#include "platform.h"
#include "sound.h"

typedef struct gl_window_t {
	Application* app;
	SDL_Window* window;
	SDL_GLContext context;
	bool isVisible;
	bool quit;
	uint64_t lastTickCount;
} GLWindow;

static bool mapKey(SDL_Keycode key, uint8_t* out)
{
	if (key >= SDLK_a && key <= SDLK_z) {
		*out = (uint8_t)('A' + (key - SDLK_a));
		return true;
	}
	if (key >= SDLK_0 && key <= SDLK_9) {
		*out = (uint8_t)key;
		return true;
	}

	switch (key) {
	case SDLK_ESCAPE: *out = VK_ESCAPE; return true;
	case SDLK_RETURN:
	case SDLK_KP_ENTER: *out = VK_RETURN; return true;
	case SDLK_LEFT: *out = VK_LEFT; return true;
	case SDLK_RIGHT: *out = VK_RIGHT; return true;
	case SDLK_UP: *out = VK_UP; return true;
	case SDLK_DOWN: *out = VK_DOWN; return true;
	case SDLK_F1: *out = VK_F1; return true;
	default:
		return false;
	}
}

static void handleEvent(GLWindow* window, const SDL_Event* event)
{
	switch (event->type) {
	case SDL_QUIT:
		window->quit = true;
		break;

	case SDL_WINDOWEVENT:
		if (event->window.event == SDL_WINDOWEVENT_MINIMIZED) {
			window->isVisible = false;
		} else if (event->window.event == SDL_WINDOWEVENT_RESTORED ||
			event->window.event == SDL_WINDOWEVENT_SHOWN ||
			event->window.event == SDL_WINDOWEVENT_MAXIMIZED) {
			window->isVisible = true;
			glDrawResize(appGetWidth(window->app), appGetHeight(window->app));
		} else if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
			glDrawResize(event->window.data1, event->window.data2);
		}
		break;

	case SDL_KEYDOWN:
	case SDL_KEYUP: {
		uint8_t vk = 0;
		if (mapKey(event->key.keysym.sym, &vk)) {
			inputKeyUpdate(vk, event->type == SDL_KEYDOWN);
		}
		break;
	}

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP: {
		InputButton button;
		bool known = true;
		if (event->button.button == SDL_BUTTON_LEFT) {
			button = INPUT_BUTTON_LEFT;
		} else if (event->button.button == SDL_BUTTON_RIGHT) {
			button = INPUT_BUTTON_RIGHT;
		} else {
			known = false;
		}
		if (known) {
			inputMouseUpdateButton(button, event->type == SDL_MOUSEBUTTONDOWN);
		}
		break;
	}

	case SDL_MOUSEMOTION: {
		Coord2D coord;
		coord.x = (float)event->motion.x;
		coord.y = (float)event->motion.y;
		inputMouseUpdatePosition(coord);
		break;
	}

	default:
		break;
	}
}

void platformShowCursor(int show)
{
	SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
}

GLWindow* fwInitWindow(Application* app)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		return NULL;
	}

	soundInit(appGetMaxSounds(app));
	inputInit();

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

	GLWindow* window = calloc(1, sizeof(GLWindow));
	if (window == NULL) {
		return NULL;
	}
	window->app = app;

	window->window = SDL_CreateWindow(
		appGetTitle(app),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		(int)appGetWidth(app),
		(int)appGetHeight(app),
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
	);
	if (window->window == NULL) {
		fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
		fwShutdownWindow(window);
		return NULL;
	}

	window->context = SDL_GL_CreateContext(window->window);
	if (window->context == NULL) {
		fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
		fwShutdownWindow(window);
		return NULL;
	}

	SDL_GL_SetSwapInterval(1);
	window->isVisible = true;
	window->lastTickCount = SDL_GetTicks64();

	glDrawInit(0.0f, 0.0f, 0.0f);
	glDrawResize(appGetWidth(app), appGetHeight(app));
	return window;
}

bool fwUpdateWindow(GLWindow* window)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		handleEvent(window, &event);
	}

	if (window->quit) {
		return false;
	}

	if (window->isVisible) {
		uint64_t now = SDL_GetTicks64();
		uint32_t ticks = (uint32_t)(now - window->lastTickCount);
		window->lastTickCount = now;

		appUpdate(window->app, ticks);
		glDrawStart();
		appDraw(window->app);
		glDrawEnd();
		SDL_GL_SwapWindow(window->window);
	}

	return true;
}

void fwShutdownWindow(GLWindow* window)
{
	inputShutdown();
	soundShutdown();

	if (window == NULL) {
		SDL_Quit();
		return;
	}

	if (window->context != NULL) {
		SDL_GL_DeleteContext(window->context);
	}
	if (window->window != NULL) {
		SDL_DestroyWindow(window->window);
	}
	free(window);
	SDL_Quit();
}

void fwSendTerminate(GLWindow* window)
{
	window->quit = true;
}

void fwSendFullscreen(GLWindow* window, bool fullscreen)
{
	SDL_SetWindowFullscreen(window->window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

bool fwChangeResolution(GLWindow* window, uint32_t width, uint32_t height, uint32_t bitsPerPixel)
{
	(void)bitsPerPixel;
	SDL_SetWindowSize(window->window, (int)width, (int)height);
	glDrawResize((int32_t)width, (int32_t)height);
	return true;
}
