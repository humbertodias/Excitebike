#include "ramp.h"
#include "rampConstants.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "collision.h"
#include "object.h"
#include "objmgr.h"
#include "player.h"
#include "SOIL.h"
#include "sprite.h"

#include "debug.h"
#include "sound.h"

typedef struct ramp_t
{
	Object 	obj;
	RampType type;
	Vector2 	initialPosition;

	Collider	collider;
}Ramp;

static void _rampDraw(Object* obj);
static void _rampUpdate(Object* obj, uint32_t milliseconds);
static void _rampOnCollision(Collision* collision, uint32_t milliseconds);
static void _rampOnTrigger(Collision* collision, uint32_t milliseconds);
static void _rampOnTriggerExit(Collision* collision, uint32_t milliseconds);
static void _rampOnCollisionEnter(Collision* collision, uint32_t milliseconds);

static ObjVtable _rampVtable = {
	_rampDraw,
	_rampUpdate,
	_rampOnCollision,
	_rampOnTrigger,
	_rampOnCollisionEnter,
	NULL,
	NULL,
	_rampOnTriggerExit
};

static const float lanes[] = { 751, 697, 643, 589 };

static GLuint rampSpriteSheet;
static const Vector2 spriteSize = { 3294, 366 };
static const char RAMP_PAGE[] = "assets/ramps2.png";

static int32_t rampSfx;
static int32_t rampSfxTimer = 0;
static int32_t rampSfxCooldown = 2000;

void rampInitTextures()
{
	rampSpriteSheet = SOIL_load_OGL_texture(RAMP_PAGE, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	assert(rampSpriteSheet != 0);

	glBindTexture(GL_TEXTURE_2D, rampSpriteSheet);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	rampSfx = soundLoad("assets/sound/ramp.wav");
}

Ramp* rampNew(float xPosition, RampType rampType)
{
	Ramp* ramp = malloc(sizeof(Ramp));
	if (ramp != NULL)
	{
		Coord2D vel = { 0.0f, 0.0f };
		Vector2 position;
		position.x = xPosition;
		position.y = lanes[0];

		objInit(&ramp->obj, &_rampVtable, (Coord2D) { position.x, position.y }, vel); // TODO change coord2d to vector2

		Shape* shape = malloc(sizeof(Shape));
		shape->type = SHAPE_TRAPEZOID;
		shape->trapezoid = malloc(sizeof(ShapeTrapezoid));

		shape->trapezoid->a = rampShapes[rampType].a;
		shape->trapezoid->b = rampShapes[rampType].b;
		shape->trapezoid->c = rampShapes[rampType].c;
		shape->trapezoid->d = rampShapes[rampType].d;

		shape->trapezoid->a = vector2Add(shape->trapezoid->a, (Vector2){ xPosition, lanes[0] + 54 });
		shape->trapezoid->b = vector2Add(shape->trapezoid->b, (Vector2){ xPosition, lanes[0] + 54});
		shape->trapezoid->c = vector2Add(shape->trapezoid->c, (Vector2){ xPosition, lanes[0] + 54 });
		shape->trapezoid->d = vector2Add(shape->trapezoid->d, (Vector2){ xPosition, lanes[0] + 54});

		ramp->collider.shape = shape;

		ramp->collider.laneSpan = laneSpan[rampType];

		ramp->collider.obj = malloc(sizeof(Object*));
		ramp->collider.obj = &ramp->obj;
		ramp->collider.isTrigger = false;
		ramp->collider.isColliding = false;
		ramp->type = rampType;
		ramp->obj.tag = "_ramp";
	}
	return ramp;
}

void rampDelete(Ramp* ramp)
{
	objDeinit(&ramp->obj);

	free(ramp);
}

static void _rampDraw(Object* obj)
{
	Ramp* ramp = (Ramp*)obj;
	if( G_DEBUG && ramp->collider.isColliding)
	{
		shapeDrawTrapezoid(ramp->collider.shape->trapezoid, 0, 0, 255);

		glColor3ub(255, 0, 0);
	}
	spriteDrawFromSheet(rampSpriteSheet, (Vector2) { obj->position.x, obj->position.y - 312 }, spriteSize, (Vector2) { (float)366 * ramp->type, 0 }, (Vector2) { 366, 366 }, 1, 0.7f, false);
	glColor3ub(255, 255, 255);



}

void _rampUpdate(Object* obj, uint32_t milliseconds)
{
	Ramp* ramp = (Ramp*)obj;

	float deltaTime = milliseconds / 1000.0f;
	float xVelocity = playerGetVelocityX();

	Vector2 transform;
	transform.x = -xVelocity * deltaTime;
	transform.y = 0;
	shapeMove(ramp->collider.shape, transform);

	Object* player = objMgrGetObjByTag("_player");		// I shouldn't NEED to do this, unsure what the better way is...

	uint8_t laneIdx = playerGetLaneIdx((Player*)player);

	float centerY = (ramp->collider.shape->trapezoid->a.y + ramp->collider.shape->trapezoid->b.y + ramp->collider.shape->trapezoid->c.y + ramp->collider.shape->trapezoid->d.y) / 4;
	Vector2 newPosition = vector2Sub((Vector2) { 0, lanes[laneIdx] + 54 }, (Vector2) {0, ramp->collider.shape->trapezoid->a.y});

	if( ramp->type == RAMP_H && (laneIdx == 0 || laneIdx == 1) )
	{
		newPosition = vector2Sub((Vector2) { 0, 5000 }, (Vector2) { 0, ramp->collider.shape->trapezoid->a.y });
	}

	shapeMove(ramp->collider.shape, newPosition);


	Collider* playerCollider = objGetCollider(player);
	Collision collision;

	ramp->collider.isColliding = false;

	ramp->collider.obj = obj;	// TODO: Find where the obj reference is getting lost after Collide

	Collide(&ramp->collider, playerCollider, &collision, milliseconds);

	Collider* playerTrigger = playerGetTrigger(player);
	Collide(&ramp->collider, playerTrigger, &collision, milliseconds);

	ShapeTrapezoid* trapezoid = ramp->collider.shape->trapezoid;
	obj->position.x = ramp->collider.shape->trapezoid->a.x;
	//obj->position.x = trapezoid->d.x + (trapezoid->d.x - trapezoid->a.x) / 2;
	//obj->position.y = rect->position.y - rect->size.y / 2;
	rampSfxTimer -= milliseconds;
	if (rampSfxTimer <= 0 && ramp->collider.isColliding)
	{
		//soundPlay(rampSfx); TODO: fix ramp audio
		rampSfxTimer = rampSfxCooldown;
	}
}

void _rampOnCollision(Collision* collision, uint32_t milliseconds)
{
	Ramp* ramp = (Ramp*)collision->self->obj;

	if (ramp->type == RAMP_H)
	{
		Vector2 velocity = collision->other->shape->velocity;
		collision->other->shape->velocity.x += velocity.x * 5 * milliseconds / 1000.0f;
		collision->other->shape->velocity.y += velocity.y * 2.5f * milliseconds / 1000.0f;
	}
}


void _rampOnTrigger(Collision* collision, uint32_t milliseconds)
{

}


void _rampOnTriggerExit(Collision* collision, uint32_t milliseconds)
{
	Ramp* ramp = (Ramp*)collision->self->obj;
	Object* player = collision->other->obj;
	Collider* playerCollider = objGetCollider(player);
	if( ramp->type == RAMP_D && playerCollider->shape->velocity.y > 0 )
	{
		playerCollider->shape->velocity.y *= 1.5f;
	}
}



void _rampOnCollisionEnter(Collision* collision, uint32_t milliseconds)
{

}
