#include "surface.h"

#include <assert.h>
#include <stdlib.h>

#include "background.h"
#include "collision.h"
#include "Object.h"
#include "objmgr.h"
#include "player.h"
#include "SOIL.h"
#include "sprite.h"
#include "debug.h"

typedef struct surface_t
{
	Object 	obj;
	SurfaceType type;
	Vector2 	initialPosition;

	Collider	collider;
	bool		collidingLastFrame;

	bool		_speedBumpHit;
}Surface;

typedef struct spriteData_t
{
	Vector2 position;
	Vector2 size;
	Vector2 offset;
}spriteData;

static void _surfaceDraw(Object* obj);
static void _surfaceUpdate(Object* obj, uint32_t milliseconds);
static void _surfaceOnCollision(Collision* collision, uint32_t milliseconds);
static void _surfaceOnTrigger(Collision* collision, uint32_t milliseconds);
static void _surfaceOnTriggerEnter(Collision* collision, uint32_t milliseconds);
static bool gameWon = false;

static ObjVtable _surfaceVtable = {
	_surfaceDraw,
	_surfaceUpdate,
	_surfaceOnCollision,
	_surfaceOnTrigger,
	NULL,
	_surfaceOnTriggerEnter,
};
												// 0 for win handling
static const float lanes[] = { 589, 643, 697, 751, 0 };
static const spriteData sprites[] =
{
	{{0, 0}, {37, 145}, {0, -37}}, 	// Bump
	{{47, 0}, {80, 40}, {0, 0}}, 		// Cooldown
	{{47, 49},{119, 55},{0, 0}},		// Dirt
	{{0, 226},{911, 109},{0, 0}},		// Grass Single 
	{{176, 0},{415, 216},{0, 0}},		// Grass Double
	{{176, 0},{415, 1080},{0, 0}}		// Win
};

static GLuint surfSpriteSheet;
const Vector2 spriteSize = {911, 335};
static const char SURFACE_PAGE[] = "asset/surfaces.png";

void surfaceInitTextures()
{
	surfSpriteSheet = SOIL_load_OGL_texture(SURFACE_PAGE, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	assert(surfSpriteSheet != 0);

	glBindTexture(GL_TEXTURE_2D, surfSpriteSheet);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Surface* surfaceNew(float xPosition, SurfaceType surfaceType, uint8_t laneStart)
{
	Surface* surface = malloc(sizeof(Surface));
	if (surface != NULL)
	{
		Coord2D vel = { 0.0f, 0.0f };
		Vector2 position;
		position.x = xPosition;
		position.y = lanes[laneStart];

		if( surfaceType == SURFACE_COOLDOWN && laneStart != 0)
		{
			position.y += 14;
		}

		if( surfaceType == SURFACE_CHECKER_WIN)
		{
			gameWon = false;
		}

		objInit(&surface->obj, &_surfaceVtable, (Coord2D) { position.x, position.y }, vel); // TODO change coord2d to vector2

		Shape* shape = malloc(sizeof(Shape));
		shape->type = SHAPE_RECTANGLE;
		shape->rectangle = malloc(sizeof(ShapeRectangle));
		shape->rectangle->size = sprites[surfaceType].size;

		if( surfaceType == SURFACE_BUMP)
		{
			shape->rectangle->size.y = 109;

		}
		shape->rectangle->position = (Vector2){ position.x, shape->rectangle->size.y / 2 + position.y };

		surface->collider.shape = shape;
		surface->collider.obj = malloc(sizeof(Object*));
		surface->collider.obj = &surface->obj;
		surface->collider.isTrigger = true;
		surface->collider.isColliding = false;
		surface->type = surfaceType;
		surface->obj.tag = "_surface";
		surface->collidingLastFrame = false;
		surface->_speedBumpHit = false;
	}
	return surface;
}

void surfaceDelete(Surface* surface)
{
	objDeinit(&surface->obj);

	free(surface);
}

static void _surfaceDraw(Object* obj)
{
	Surface* surface = (Surface*)obj;
	if (surface->type == SURFACE_CHECKER_WIN) return;
	spriteData sd = sprites[surface->type];
	if (G_DEBUG && surface->collider.isColliding)
	{
		glColor3ub(255, 0, 0);
	}
	spriteDrawFromSheet(surfSpriteSheet, (Vector2) { obj->position.x + sd.offset.x, obj->position.y + sd.offset.y }, spriteSize, sd.position, sd.size, 1, 0.75f, false);

	//ShapeRectangle* rect = surface->collider.shape->rectangle;
	//Vector2 topLeft = { rect->position.x - (rect->size.x / 2), rect->position.y - (rect->size.y / 2) };
	//Vector2 botRight = { rect->position.x + (rect->size.x / 2), rect->position.y + (rect->size.y / 2) };
	//shapeDrawRect(topLeft, botRight, 255, 255, 255, 2, true);
	glColor3ub(255, 255, 255);
}

void _surfaceUpdate(Object* obj, uint32_t milliseconds)
{
	Surface* surface = (Surface*)obj;

	float deltaTime = milliseconds / 1000.0f;
	float xVelocity = playerGetVelocityX();
	Vector2 transform;
	transform.x = -xVelocity * deltaTime;
	transform.y = 0;
	shapeMove(surface->collider.shape, transform);

	Object* player = objMgrGetObjByTag("_player");		// I shouldn't NEED to do this, unsure what the better way is...
	Collider* playerCollider = objGetCollider(player);
	Collision collision;
	surface->collider.isColliding = false;
	//playerResetTopSpeed((Player*)player);

	surface->collider.obj = obj;	// TODO: Find where the obj reference is getting lost after Collide
	if( !surface->_speedBumpHit)
	{
		if (Collide(&surface->collider, playerCollider, &collision, milliseconds))
		{
			surface->collidingLastFrame = true;
		}
		else if (surface->collidingLastFrame)
		{
			playerResetTopSpeed((Player*)player);
			surface->collidingLastFrame = false;
		}
	}

	if (surface->collider.shape->rectangle->position.x + surface->collider.shape->rectangle->size.x < 0)
	{
		//surface->collider.shape->rectangle->position.x = 2000;
	}
	ShapeRectangle* rect = surface->collider.shape->rectangle;
	obj->position.x = rect->position.x - rect->size.x / 2;
	//obj->position.y = rect->position.y - rect->size.y / 2;

}

void _surfaceOnCollision(Collision* collision, uint32_t milliseconds)
{

}


void _surfaceOnTrigger(Collision* collision, uint32_t milliseconds)
{
	Surface* surface = (Surface*)collision->self->obj;
	Object* otherObj = collision->other->obj;
	if (!objCompareTag(otherObj, "_player")) return; // Return if not player

	Player* player = (Player*)otherObj;

	switch (surface->type)
	{
		case SURFACE_BUMP:
			if (!playerIsWheelie(player))
			{
				playerCrash(player);
				surface->_speedBumpHit = true;
			}
			break;
		case SURFACE_COOLDOWN:
			//TODO playerReduceEngineHeat(amount)
			break;
		case SURFACE_DIRT:
			playerSetTopSpeed(player, 100);
			break;
		case SURFACE_GRASS_SINGLE:
			playerSetTopSpeed(player, 200);
			break;
		case SURFACE_GRASS_DOUBLE:
			playerSetTopSpeed(player, 500);
			break;
	}
}

void _surfaceOnTriggerEnter(Collision* collision, uint32_t milliseconds)
{
	Surface* surface = (Surface*)collision->self->obj;
	if( surface->type == SURFACE_COOLDOWN)
	{
		Player* player = (Player*)collision->other->obj;
		playerCooldown(player);
	}

	if( surface->type == SURFACE_CHECKER_WIN && !gameWon)
	{
		gameWon = true;
		backgroundWin();
		playerWin();
	}
}
