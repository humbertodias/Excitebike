#include "SOIL.h"
#include "gl_compat.h"

#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <string.h>

static int s_imageReady = 0;

static void flipVertical(unsigned char* pixels, int width, int height)
{
	size_t stride = (size_t)width * 4;
	unsigned char* row = malloc(stride);
	if (row == NULL) {
		return;
	}
	for (int y = 0; y < height / 2; ++y) {
		unsigned char* top = pixels + (size_t)y * stride;
		unsigned char* bottom = pixels + (size_t)(height - 1 - y) * stride;
		memcpy(row, top, stride);
		memcpy(top, bottom, stride);
		memcpy(bottom, row, stride);
	}
	free(row);
}

unsigned int SOIL_load_OGL_texture(const char* filename, int force_channels, unsigned int reuse_texture_ID, unsigned int flags)
{
	(void)force_channels;

	if (!s_imageReady) {
		IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
		s_imageReady = 1;
	}

	SDL_Surface* loaded = IMG_Load(filename);
	if (loaded == NULL) {
		return 0;
	}

	SDL_Surface* converted = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_ABGR8888, 0);
	SDL_FreeSurface(loaded);
	if (converted == NULL) {
		return 0;
	}

	int width = converted->w;
	int height = converted->h;
	unsigned char* pixels = malloc((size_t)width * (size_t)height * 4);
	if (pixels == NULL) {
		SDL_FreeSurface(converted);
		return 0;
	}

	unsigned char* src = (unsigned char*)converted->pixels;
	for (int y = 0; y < height; ++y) {
		memcpy(pixels + (size_t)y * (size_t)width * 4, src + (size_t)y * (size_t)converted->pitch, (size_t)width * 4);
	}
	SDL_FreeSurface(converted);

	if (flags & SOIL_FLAG_INVERT_Y) {
		flipVertical(pixels, width, height);
	}

	GLuint texture = reuse_texture_ID;
	if (texture == 0) {
		glGenTextures(1, &texture);
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	if (flags & SOIL_FLAG_MIPMAPS) {
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLint wrap = (flags & SOIL_FLAG_TEXTURE_REPEATS) ? GL_REPEAT : GL_CLAMP;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	free(pixels);
	return texture;
}
