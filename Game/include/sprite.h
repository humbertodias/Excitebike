#pragma once
#include <Windows.h>
#include <gl/GLU.h>

#include "baseTypes.h"
#include "vector.h"


// Sprite data
typedef struct sprite_t {
	GLuint		spriteSheet;
	uint16_t	cols;
	uint16_t	rows;
	GLfloat		spriteWidth;
	GLfloat		spriteHeight;
} Sprite;

// Animation looping or non looping through a sprite sheet
typedef struct animation_t
{
	Sprite* sprite;
	uint8_t startFrame;
	uint8_t endFrame;
	uint8_t currFrame;
	uint8_t playbackSpeed;
	uint32_t accumulatedTime;

	bool	useFrameOrder;
	uint8_t numFrames;
	uint8_t frameIndex;
	uint8_t* frames;

	int8_t  numLoops;

	bool 	bPlaying;

}Animation;

// Play multiple animations in a row
typedef struct sequence_t
{
	Animation* 	animationList;
	uint8_t		numAnimations;
	uint8_t		animationIndex;

}Sequence;

Sprite* spriteNew(const char* spriteSheetFileName, uint16_t rows, uint16_t cols, GLfloat spriteWidth, GLfloat spriteHeight);
void spriteDraw(const Sprite* sprite, Coord2D position, float scale, int frame, float depth, bool alignCenter);
void spriteDrawFromSheet(GLuint spriteSheet, Vector2 position, Vector2 size, Vector2 uvStart, Vector2 uvSize, float scale, float depth, bool alignCenter);
bool playAnimation(Animation* animation, uint32_t milliseconds);
void stopAnimation(Animation* animation);