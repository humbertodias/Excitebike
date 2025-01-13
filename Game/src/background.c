#include "background.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "collision.h"
#include "Object.h"
#include "player.h"
#include "sprite.h"

typedef struct background_t
{
	Object 	obj;
}Background;

static void _backgroundDraw(Object* obj);
static void _backgroundUpdate(Object* obj, uint32_t milliseconds);

static ObjVtable _backgroundVtable = {
	_backgroundDraw,
	_backgroundUpdate,
};

static Sprite* bgSprite;
static Sprite* crowdSprite;


static float scrollOffsetU = 0.0f; // Horizontal scroll offset
static float scrollOffsetV = 0.0f; // Vertical scroll offset

static float new_scrollOffsetV = 0.0f;
static float new_scrollOffsetU = 0.0f;

static float SCROLL_SPEED_U = 0.001f; // Speed for U-axis (horizontal)
static const float SCROLL_SPEED_V = 0.0f; // Speed for V-axis (vertical)

static int winTimer = 100;
static int winCooldown = 50;

static bool gameWon = false;
uint8_t frame = 4;

void backgroundInitTextures()
{
	bgSprite = spriteNew("asset/background.png", 1, 1, 1143, 1080);
	crowdSprite = spriteNew("asset/crowd.png", 5, 1, 2303, 288);
}

Background* backgroundNew()
{
	Background* background = malloc(sizeof(Background));
	if (background != NULL)
	{
		Coord2D vel = { 0.0f, 0.0f };

		objInit(&background->obj, &_backgroundVtable, (Coord2D) {0, 0 }, vel); // TODO change coord2d to vector2
	}
	gameWon = false;
	frame = 4; // Set frame to default crowd
	return background;
}

void backgroundDelete(Background* background)
{
	objDeinit(&background->obj);

	free(background);
}


static void _backgroundDraw(Object* obj)
{


		// Update scrolling offset

		scrollOffsetU += SCROLL_SPEED_U;
		scrollOffsetV += SCROLL_SPEED_V;

		// Keep UV coordinates within range [0, 1]
		scrollOffsetU = (float)fmod(scrollOffsetU, 1.0f);
		scrollOffsetV = (float)fmod(scrollOffsetV, 1.0f);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, bgSprite->spriteSheet);

		// Ensure texture wrapping mode is set to repeat
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBegin(GL_TRIANGLE_STRIP);

		// Calculate the bounding box
		GLfloat xPositionLeft = 0.0f;
		GLfloat xPositionRight = 1143.0f;
		GLfloat yPositionTop = 0.0f;
		GLfloat yPositionBottom = 1080;

		float BG_DEPTH = 0.0f;

		// Set vertex colors to white (fully opaque)
		glColor4ub(255, 255, 255, 255);

		// Apply the scrolling offsets to texture coordinates
		// Top-left
		glTexCoord2f(scrollOffsetU, 1.0f + scrollOffsetV);
		glVertex3f(xPositionLeft, yPositionTop, BG_DEPTH);

		// Bottom-left
		glTexCoord2f(scrollOffsetU, scrollOffsetV);
		glVertex3f(xPositionLeft, yPositionBottom, BG_DEPTH);

		// Top-right
		glTexCoord2f(1.0f + scrollOffsetU, 1.0f + scrollOffsetV);
		glVertex3f(xPositionRight, yPositionTop, BG_DEPTH);

		// Bottom-right
		glTexCoord2f(1.0f + scrollOffsetU, scrollOffsetV);
		glVertex3f(xPositionRight, yPositionBottom, BG_DEPTH);

		glEnd();

		// Keep UV coordinates within range [0, 1]
		new_scrollOffsetU += SCROLL_SPEED_U / 1.75f;
		new_scrollOffsetV += SCROLL_SPEED_V;
		new_scrollOffsetU = (float)fmod(new_scrollOffsetU, 1.0f);
		new_scrollOffsetV = (float)fmod(new_scrollOffsetV, 1.0f);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, crowdSprite->spriteSheet);

		// Ensure texture wrapping mode is set to repeat
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBegin(GL_TRIANGLE_STRIP);

		// Calculate the bounding box
		xPositionLeft = 0.0f;
		xPositionRight = 2303;
		yPositionTop = 0.0f;
		yPositionBottom = 288;

		BG_DEPTH = 0.0f;
		GLfloat rowHeight = 1.0f / crowdSprite->rows;
		GLfloat yTextureCoord = frame * rowHeight;
		// Set vertex colors to white (fully opaque)
		glColor4ub(255, 255, 255, 255);

		// Apply the scrolling offsets to texture coordinates
		// Top-left
		glTexCoord2f(new_scrollOffsetU, yTextureCoord + rowHeight);
		glVertex3f(xPositionLeft, yPositionTop, BG_DEPTH);

		// Bottom-left
		glTexCoord2f(new_scrollOffsetU, yTextureCoord);
		glVertex3f(xPositionLeft, yPositionBottom, BG_DEPTH);

		// Top-right
		glTexCoord2f(1.0f + new_scrollOffsetU, yTextureCoord + rowHeight);
		glVertex3f(xPositionRight, yPositionTop, BG_DEPTH);

		// Bottom-right
		glTexCoord2f(1.0f + new_scrollOffsetU, yTextureCoord);
		glVertex3f(xPositionRight, yPositionBottom, BG_DEPTH);

		glEnd();
	
}

void _backgroundUpdate(Object* obj, uint32_t milliseconds)
{
	Background* background = (Background*)obj;

	SCROLL_SPEED_U = playerGetVelocityX() / 1143 * milliseconds / 1000;

	if( gameWon )
	{
		winTimer -= milliseconds;
		if (winTimer < 0) // Cycle crowd frame to flash
		{
			winTimer = winCooldown;
			++frame;
			if (frame == 5) frame = 0;
		}
	}
}

void backgroundWin()
{
	gameWon = true;
}