
#include "baseTypes.h"
#include "sprite.h"

#include <assert.h>

#include "SOIL.h"
#include "vector.h"

bool _AnimCustomFrameOrder(Animation* animation, uint32_t milliseconds);

Sprite* spriteNew(const char* spriteSheetFileName, uint16_t rows, uint16_t cols, GLfloat spriteWidth, GLfloat spriteHeight)
{
	Sprite* sprite = malloc(sizeof(Sprite));
	if( sprite != NULL)
	{
		sprite->spriteSheet = SOIL_load_OGL_texture(spriteSheetFileName, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		assert(sprite->spriteSheet != 0);

		sprite->rows = rows;
		sprite->cols = cols;
		sprite->spriteWidth = spriteWidth;
		sprite->spriteHeight = spriteHeight;
		glBindTexture(GL_TEXTURE_2D, sprite->spriteSheet);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	return sprite;
}

void spriteDraw(const Sprite* sprite, Coord2D position, float scale, int frame, float depth, bool alignCenter)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, sprite->spriteSheet);

	// Calculate the bounding box for the sprite

	GLfloat xPositionLeft;
	GLfloat xPositionRight;
	GLfloat yPositionTop;
	GLfloat yPositionBottom;

	if( alignCenter )
	{
		xPositionLeft = position.x - sprite->spriteWidth / 2 * scale;
		xPositionRight = position.x + sprite->spriteWidth / 2 * scale;
		yPositionTop = position.y - sprite->spriteHeight / 2 * scale;
		yPositionBottom = position.y + sprite->spriteHeight / 2 * scale;
	}else
	{
		xPositionLeft = position.x;
		xPositionRight = position.x + sprite->spriteWidth * scale;
		yPositionTop = position.y;
		yPositionBottom = position.y + sprite->spriteHeight * scale;
	}

	// Calculate texture coordinates
	GLfloat columnWidth = 1.0f / sprite->cols;
	GLfloat rowHeight = 1.0f / sprite->rows;
	
	// Calculate the current column and row
	int col = frame % sprite->cols;                       // Column index
	int row = sprite->rows - 1 - (frame / sprite->cols);  // Inverted row index (top-to-bottom)

	// Get UV coordinates for the current frame
	GLfloat xTextureCoord = col * columnWidth;
	GLfloat yTextureCoord = row * rowHeight;           // Top edge of the frame

	// Draw the textured quad (tristrip)
	glColor4ub(0xFF, 0xFF, 0xFF, 0xFF); // White color (no tint)

	glBegin(GL_TRIANGLE_STRIP);

	// Top-left corner
	glTexCoord2f(xTextureCoord, yTextureCoord + rowHeight);
	glVertex3f(xPositionLeft, yPositionTop, depth);

	// Bottom-left corner
	glTexCoord2f(xTextureCoord, yTextureCoord);
	glVertex3f(xPositionLeft, yPositionBottom, depth);

	// Top-right corner
	glTexCoord2f(xTextureCoord + columnWidth, yTextureCoord + rowHeight);
	glVertex3f(xPositionRight, yPositionTop, depth);

	// Bottom-right corner
	glTexCoord2f(xTextureCoord + columnWidth, yTextureCoord);
	glVertex3f(xPositionRight, yPositionBottom, depth);
	glEnd();
}


void spriteDrawFromSheet(GLuint spriteSheet, Vector2 position, Vector2 size, Vector2 uvStart, Vector2 uvSize,
	float scale, float depth, bool alignCenter)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, spriteSheet);
	// Convert UVs from pixel coordinates to normalized coordinates
	float u1 = uvStart.x / size.x;
	float v1 = 1.0f - (uvStart.y / size.y);                // Flip the vertical axis
	float u2 = (uvStart.x + uvSize.x) / size.x;
	float v2 = 1.0f - ((uvStart.y + uvSize.y) / size.y);   // Flip the vertical axis

	// Calculate the bounding box for the sprite
	GLfloat xPositionLeft, xPositionRight, yPositionTop, yPositionBottom;

	if (alignCenter)
	{
		xPositionLeft = position.x - uvSize.x / 2 * scale;
		xPositionRight = position.x + uvSize.x / 2 * scale;
		yPositionTop = position.y - uvSize.y / 2 * scale;
		yPositionBottom = position.y + uvSize.y / 2 * scale;
	}
	else
	{
		xPositionLeft = position.x;
		xPositionRight = position.x + uvSize.x * scale;
		yPositionTop = position.y;
		yPositionBottom = position.y + uvSize.y * scale;
	}

	// Begin drawing a textured quad
	glBegin(GL_QUADS);

	// Bottom-left
	glTexCoord2f(u1, v2);  // Use v2 for bottom
	glVertex3f(xPositionLeft, yPositionBottom, depth);

	// Bottom-right
	glTexCoord2f(u2, v2);  // Use v2 for bottom
	glVertex3f(xPositionRight, yPositionBottom, depth);

	// Top-right
	glTexCoord2f(u2, v1);  // Use v1 for top
	glVertex3f(xPositionRight, yPositionTop, depth);

	// Top-left
	glTexCoord2f(u1, v1);  // Use v1 for top
	glVertex3f(xPositionLeft, yPositionTop, depth);

	glEnd();
}






bool playAnimation(Animation* animation, uint32_t milliseconds)
{
	if (!animation || !animation->sprite || !animation->bPlaying)
		return false; 

	if( animation->useFrameOrder )
	{
		return _AnimCustomFrameOrder(animation, milliseconds);
	}

	static uint32_t accumulatedTime = 0;
	accumulatedTime += milliseconds;

	uint32_t frameDuration = animation->playbackSpeed;

	while (accumulatedTime >= frameDuration)
	{
		accumulatedTime -= frameDuration;

		animation->currFrame++;

		// Check if we've reached the end of the animation
		if (animation->currFrame > animation->endFrame)
		{
			if (animation->numLoops > 0)
			{
				// Decrement loops and reset to startFrame
				animation->numLoops--;
				animation->currFrame = animation->startFrame;
			}
			else if (animation->numLoops == 0)
			{
				// Stop animation if no loops are remaining
				animation->bPlaying = false;
				return false;
			}
			else
			{
				// Infinite loop, reset to startFrame
				animation->currFrame = animation->startFrame;
			}
		}
	}

	return true; // Animation is still playing
}

bool _AnimCustomFrameOrder(Animation* animation, uint32_t milliseconds)
{
	animation->accumulatedTime += milliseconds;

	uint32_t frameDuration = animation->playbackSpeed;
	animation->currFrame = animation->frames[animation->frameIndex];
	while (animation->accumulatedTime >= frameDuration)
	{
		animation->accumulatedTime -= frameDuration;

		animation->frameIndex++;

		// Check if we've reached the end of the animation
		if (animation->frameIndex == animation->numFrames )
		{
			animation->accumulatedTime = 0;
			animation->frameIndex = 0;
			if (animation->numLoops > 0)
			{
				// Decrement loops and reset
				animation->numLoops--;
			}
			else if (animation->numLoops == 0)
			{
				// Stop animation if no loops are remaining
				animation->bPlaying = false;
				return false;
			}
		}
		animation->currFrame = animation->frames[animation->frameIndex];

	}

	return true; // Animation is still playing
}

void stopAnimation(Animation* animation)
{
	animation->accumulatedTime = 0;
	animation->frameIndex = 0;
	animation->bPlaying = false;
}
