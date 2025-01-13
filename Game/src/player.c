#include <Windows.h>
#include <stdlib.h>
#include <assert.h>
#include <gl/GLU.h>

#include "debug.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include "SOIL.h"
#include "player.h"
#include "input.h"
#include "Object.h"
#include "shape.h"
#include "sprite.h"
#include "collision.h"
#include "game.h"
#include "objmgr.h"
#include "sound.h"
#include "vector.h"

bool G_DEBUG = false;
const float AIR_SLOWDOWN = 750.0f; // Air resistance when over angle > 45d
const float GRAVITY = 800;
const uint32_t lanes[] = { 758, 705, 652, 599 };
const float playerBrakeForce = 700;
const uint8_t laneCount = 4;

static const char PLAYER_PAGE[] = "asset/moto_sprites.png";

static Sprite* uiSprite = NULL;
static Sprite* overheatSprite = NULL;
static Sprite* numberSprite = NULL;

static int32_t overheatSfx;
static int32_t crashSfx;
static int32_t winSfx;
static int32_t idleSfx;
static int32_t idleSfxTimer = 0;
static int32_t idleSfxTimerCooldown = 30000;
static int32_t driveSfx;
static int32_t driveSfxTimer = 0;
static int32_t DriveSfxTimerCooldown = 30000;

static float playerVelocityX = 0;
static int32_t winCooldown = 8000;
static int32_t winTimer = 8000;

static uint32_t timeM = 0;
static uint32_t timeS = 0;
static uint32_t timeMS = 0;

typedef struct playerSpeed_t
{
	struct
	{
		float max;
		float current;
		float acceleration;
	}high;
	struct
	{
		float max;
		float current;
		float acceleration; 
	}low;
}PlayerSpeed;

static PlayerSpeed playerSpeed = { {700, 700, 300}, {600, 600, 250} };

typedef enum
{
	INPUT_KEY_TURN_LEFT = VK_UP, 	
	INPUT_KEY_TURN_RIGHT = VK_DOWN, 
	INPUT_KEY_TILT_LEFT = VK_LEFT, 	
	INPUT_KEY_TILT_RIGHT = VK_RIGHT,
	INPUT_KEY_LOW_GEAR = 'X',
	INPUT_KEY_HIGH_GEAR = 'Z'
}Keys;




typedef enum playerState_t
{
	STATE_FREEZE, // Used mainly for crash animations
	STATE_IDLE,
	STATE_DRIVING,
	STATE_WHEELIE,
	STATE_LEFT,
	STATE_RIGHT,
	STATE_CRASH,
	STATE_CRASH_BACKFLIP,
	STATE_OVERHEAT,
	STATE_ON_RAMP,
	STATE_ANIM_PLAY
} PlayerState;

typedef enum animationFrame_t
{
	F_IDLE,
	F_DRIVE_1,
	F_DRIVE_2,
	F_LEFT_TURN,
	F_RIGHT_TURN,
	F_ANGLE_15,
	F_ANGLE_30,
	F_ANGLE_45,
	F_ANGLE_60,
	F_ANGLE_75,
	F_ANGLE_90,
	F_ANGLE_120,
	F_ANGLE_180,
	F_ANGLE_225,
	F_ANGLE_270,
	F_ANGLE_345,
	F_ANGLE_330,
	F_ANGLE_315,
	F_ANGLE_285,
	F_BLANK_1,
	F_SITTING,
	F_RUN_1,
	F_RUN_2,
	F_BIKE_CRASH,
	F_BLANK_2,
	F_PICKUP_1,
	F_PICKUP_2,
	F_PICKUP_3,
	F_WIN
}PlayerAnimationFrame;



typedef struct player_t
{
	Object      obj;

	Sprite*		sprite;

	bool 		isGrounded;
	bool 		onRamp;

	float		angle;

	uint8_t 	currentLane;
	uint8_t 	targetLane;

	uint32_t    nextDriveFrameUpdate;
	uint32_t	nextWheelieFrameUpdate;
	uint32_t    nextTurnTime;
	uint32_t 	targetY;

	Collider	collider; 	// This is used for colliding with ramps
	Collider 	trigger;	// This is used for preventing the player from bouncing on ramps

	Vector2 	frontWheelOffset;
	Shape 		frontWheel;

	PlayerState state;
	PlayerAnimationFrame frame;

	Animation	crashWheelie;
	Animation	bikePickup;

	struct
	{
		float max;
		float current;
		float incRate;
		float decRate;
	}heat;

} Player;

bool debugToggle = false;
bool pressedLastFrame = false;

static GLuint _playerTexture = 0;

// the object vtable for all players
static void      _playerUpdate(Object* obj, uint32_t milliseconds);
static void      _playerDraw(Object* obj);
static void      _moveToTargetLane(Player* player, uint32_t milliseconds);
static void      _playerOnCollision(Collision* collision, uint32_t milliseconds);
static void      _playerOnTrigger(Collision* collision, uint32_t milliseconds);
static void		_playerOnTriggerExit(Collision* collision, uint32_t milliseconds);


static Collider* _playerGetCollider(Object* obj);
static void      playerHandleInput(Player* player, uint32_t milliseconds);
static void      playerSetAngle(Player* player, float angle);
void drawTime(uint32_t m, uint32_t s, uint32_t ms);


bool adjustWheelPosition(Shape* wheel, const ShapeTrapezoid trapezoid, uint32_t ms, int *direction);

static ObjVtable _playerVtable = {
	_playerDraw,
	_playerUpdate,
	_playerOnCollision,
	_playerOnTrigger,
	NULL,
	NULL,
	NULL,
	_playerOnTriggerExit,
	_playerGetCollider
};

float scale = 1.0F;
float y_Position = 0;
float x_pPosition = 0;
float width = 288.571f;
float height = 366;

static bool gameWon = false;
/// @brief Allocates & initializes a player object
/// @param box 
/// @return 
Player* playerNew(Bounds2D box)
{
	Player* player = malloc(sizeof(Player));
	if (player != NULL)
	{
		Coord2D center = boundsGetCenter(&box);
		Coord2D vel = { 0.0f, 0.0f };
		objInit(&player->obj, &_playerVtable, center, vel);

		player->obj.position = (Coord2D){ 200, 520 };
		player->angle = 0;

		player->nextDriveFrameUpdate = 50;
		player->nextWheelieFrameUpdate = 50;
		player->currentLane = 0;
		player->targetLane = 0;

		player->obj.velocity.y = 250;
		player->obj.velocity.x = 0;

		player->frontWheelOffset = (Vector2){ 0, 20 };

		ShapeCircle *circle = malloc(sizeof(ShapeCircle));
		circle->center = (Vector2){ player->obj.position.x + player->frontWheelOffset.x, player->obj.position.y + player->frontWheelOffset.y };
		circle->radius = 15.0f;

		player->frontWheel.type = SHAPE_CIRCLE;

		player->frontWheel.circle = circle;

		player->sprite = malloc(sizeof(Sprite*));
		player->sprite = spriteNew(PLAYER_PAGE, 6, 5, 77, 84);

		uiSprite = malloc(sizeof(Sprite*));
		uiSprite = spriteNew("asset/infoPanel.png", 1, 1, 1143, 202);

		overheatSprite = malloc(sizeof(Sprite*));
		overheatSprite = spriteNew("asset/text.png", 1, 1, 643, 42);

		numberSprite = malloc(sizeof(Sprite*));
		numberSprite = spriteNew("asset/numbers.png", 1, 10, 28, 28 );

		player->isGrounded = false;
		player->onRamp = false;

		player->collider.obj = malloc(sizeof(Object*));
		player->collider.obj = &player->obj;
		player->collider.shape = &player->frontWheel;
		player->collider.isTrigger = false;

		Shape* shape = malloc(sizeof(Shape));
		ShapeCircle* circle2 = malloc(sizeof(ShapeCircle));
		circle2->center = circle->center;
		circle2->radius = circle->radius * 2.0f;

		shape->type = SHAPE_CIRCLE;

		shape->circle = circle2;

		player->trigger.obj = malloc(sizeof(Object*));
		player->trigger.obj = &player->obj;

		player->trigger.shape = malloc(sizeof(Shape*));
		player->trigger.shape = shape;
		player->trigger.isTrigger = true;
		
		player->heat.incRate = 10;
		player->heat.decRate = 5;

		player->heat.current = 0;
		player->heat.max = 100;


		player->crashWheelie.bPlaying = false;
		player->crashWheelie.playbackSpeed = 50;
		player->crashWheelie.frameIndex = 0;
		player->crashWheelie.numFrames = 24;
		player->crashWheelie.frames = malloc(sizeof(uint8_t) * player->crashWheelie.numFrames);
		uint8_t frameOrder[] = { 11, 12, 13, 14, 18, 17, 16, 15, 1, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 18, 17, 16, 15, 1 };

		for (int i = 0; i < player->crashWheelie.numFrames; i++) {
			player->crashWheelie.frames[i] = frameOrder[i];
		}		player->crashWheelie.useFrameOrder = true;
		player->crashWheelie.numLoops = -1;
		player->crashWheelie.accumulatedTime = 0;


		player->bikePickup.bPlaying = false;
		player->bikePickup.playbackSpeed = 100;
		player->bikePickup.frameIndex = 0;
		player->bikePickup.numFrames = 16;
		player->bikePickup.frames = malloc(sizeof(uint8_t) * player->crashWheelie.numFrames);
		uint8_t frameOrder2[] = { 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 27, 2, 2, 2 };

		for (int i = 0; i < player->bikePickup.numFrames; i++) {
			player->bikePickup.frames[i] = frameOrder2[i];
		}		player->bikePickup.useFrameOrder = true;
		player->bikePickup.numLoops = 0;
		player->bikePickup.accumulatedTime = 0;
	}

	player->frontWheel.velocity = (Vector2){ 0, 0 };
	player->frame = F_IDLE;

	player->obj.tag = "_player";

	overheatSfx = soundLoad("asset/sound/overheat.wav");
	winSfx = soundLoad("asset/sound/victory.wav");
	idleSfx = soundLoad("asset/sound/idle.wav");
	driveSfx = soundLoad("asset/sound/driving.wav");
	crashSfx = soundLoad("asset/sound/crash.wav");
	return player;
}

/// @brief Frees up resources from a player object
/// @param player 
void playerDelete(Player* player)
{
	objDeinit(&player->obj);

	free(player);
}


// Draw function
static void _playerDraw(Object* obj)
{
	Player* player = (Player*)obj;
	spriteDraw(player->sprite, (Coord2D) { player->obj.position.x, player->obj.position.y - 25 }, 1.5f, player->frame, 0.99f, true);

	if(debugToggle)
	{
		ShapeCircle* wheel1 = player->frontWheel.circle;
		if( player->onRamp)
		{
			shapeDrawCircle(player->trigger.shape->circle->radius, wheel1->center.x, wheel1->center.y, 255, 0, 0, true);

			shapeDrawCircle(wheel1->radius, wheel1->center.x, wheel1->center.y, 0, 0, 255, true);
		}
		else
		{
			shapeDrawCircle(wheel1->radius, wheel1->center.x, wheel1->center.y, 0, 0, 255, true);
		}

		if (player->currentLane != player->targetLane)
			shapeDrawLine(player->obj.position.x + 50, (float)lanes[player->targetLane] + 30, 10000, (float)lanes[player->targetLane] + 30, 255, 0, 0, 1);
		shapeDrawLine(player->obj.position.x + 50, (float)lanes[player->currentLane] + 30, 10000, (float)lanes[player->currentLane] + 30, 0, 255, 0, 1);


		if(!player->isGrounded)
		{
			shapeDrawLine(player->obj.position.x, player->obj.position.y, player->obj.position.x, player->frontWheel.circle->center.y + 20.0f, 0, 255, 255, 4);
		}

		float angleInRadians = player->angle * (float)(M_PI / 180.0f);

		// Calculate the end point of the normal line
		float endX = player->obj.position.x + (float)cos(angleInRadians) * 100;
		float endY = player->obj.position.y + -(float)sin(angleInRadians) * 100;

		// Draw the line
		shapeDrawLine(player->obj.position.x, player->obj.position.y, endX, endY, 255, 255, 0, 2);
	}

	float heatFill = (player->heat.current * 143) / player->heat.max;
	Vector2 heatBoxStart = (Vector2){ 507, 950 };
	Vector2 heatBoxEnd = (Vector2){ heatBoxStart.x + heatFill, 987 };


	spriteDraw(uiSprite,(Coord2D) { 0, 879 }, 1, 0, 1, false);
	shapeDrawRect(heatBoxStart,heatBoxEnd, 200, 68, 0, 0, true);

	drawTime(timeM, timeS, timeMS);

	if( player->state == STATE_OVERHEAT)
		spriteDrawFromSheet(overheatSprite->spriteSheet, (Vector2) { 570, 389 }, (Vector2) { 643, 42 }, (Vector2) { 0, 0 }, (Vector2) { 404, 42 }, 1, 1, true);
	//shapeDrawLine(507, 969, 700, 969, 255, 0, 0, 128);
}

int direction = -1;

/// @brief Updates the character's mood every so often
/// @param obj 
/// @param milliseconds 
static void _playerUpdate(Object* obj, uint32_t milliseconds)
{

	float deltaTime = milliseconds / 1000.0f;
	Player* player = (Player*)obj;

	if(player->state == STATE_ANIM_PLAY)
	{
		if( player->crashWheelie.bPlaying)
		{
			playAnimation(&player->crashWheelie, milliseconds);
			player->frame = player->crashWheelie.currFrame;
			if (player->frontWheel.velocity.x <= 1.0f)
			{
				stopAnimation(&player->crashWheelie);
				player->bikePickup.bPlaying = true;
			}

		}
		else if( player->bikePickup.bPlaying )
		{
			playAnimation(&player->bikePickup, milliseconds);
			player->frame = player->bikePickup.currFrame;
		}
		else
		{
			player->state = STATE_IDLE;
		}
		
		if (player->frontWheel.velocity.x > 0)
		{
			player->frontWheel.velocity.x -= playerBrakeForce * deltaTime;
			if (player->frontWheel.velocity.x < 0)
				player->frontWheel.velocity.x = 0;

		}
		else
		{
			player->targetLane = 3;
			if (player->obj.position.y > lanes[player->targetLane] - 54)
			{
				player->frontWheel.circle->center.y -= player->obj.velocity.y * deltaTime;
			}
			else
			{
				player->currentLane = player->targetLane;

			}
		}


		player->frontWheel.circle->center.y -= player->frontWheel.velocity.y * deltaTime; // TODO fix

		if (player->state != STATE_LEFT && player->state != STATE_RIGHT)
			player->obj.position.y = player->frontWheel.circle->center.y - player->frontWheelOffset.y;
	}
	else
	{
		playerHandleInput(player, milliseconds);

		if (player->nextDriveFrameUpdate > milliseconds)
		{
			player->nextDriveFrameUpdate -= milliseconds;

		}
		else if (player->state == STATE_DRIVING && player->isGrounded)
		{
			player->nextDriveFrameUpdate = 50;
			if (player->frame != F_DRIVE_2)
			{
				player->frame = F_DRIVE_2;
			}
			else
			{
				player->frame = F_DRIVE_1;
			}
		}




		if (player->currentLane != player->targetLane && player->isGrounded && player->state != STATE_OVERHEAT)
		{
			_moveToTargetLane(player, milliseconds);
		}




		player->frontWheel.circle->center.y -= player->frontWheel.velocity.y * deltaTime; // TODO fix

		if (player->state != STATE_LEFT && player->state != STATE_RIGHT)
			player->obj.position.y = player->frontWheel.circle->center.y - player->frontWheelOffset.y;

		if (player->state != STATE_OVERHEAT)
		{
			if (player->frontWheel.circle->center.y < (float)lanes[player->currentLane] + player->frontWheelOffset.y)
			{
				player->frontWheel.velocity.y -= GRAVITY * deltaTime; // 14.25
				player->isGrounded = false;

			}
			else
			{
				if (player->frame >= F_ANGLE_15 && player->frame <= F_ANGLE_285 && !player->isGrounded && !player->onRamp)
				{
					if( (player->frame > 7 && player->frame < 9)||
						(player->frame == F_ANGLE_315 || player->frame == F_ANGLE_285 ))
					{
						player->frontWheel.velocity.y = 0;
						playerCrash(player);
					}
					else
					{
						player->frontWheel.velocity.y = -player->frontWheel.velocity.y / 2;
						player->frontWheel.velocity.x = player->frontWheel.velocity.x / 1.5f;
						player->frontWheel.circle->center.y = (float)lanes[player->currentLane] + player->frontWheelOffset.y - 2; // Nudge player above ground
						player->frame = F_DRIVE_1;
					}
				}
				else
				{
					player->frontWheel.circle->center.y = (float)lanes[player->currentLane] + player->frontWheelOffset.y;
					player->frontWheel.velocity.y = 0;
					player->isGrounded = true;
				}

			}
		}


		if (!player->isGrounded && player->frame > F_ANGLE_45 && player->frame < F_ANGLE_90)
		{
			player->frontWheel.velocity.x -= AIR_SLOWDOWN * deltaTime;
			if (player->frontWheel.velocity.x < 0)
			{
				player->frontWheel.velocity.x = 0;
			}
		}

		if (player->onRamp)
		{
			if (direction == 1)
			{
				player->frame = F_ANGLE_330;
			}
			else if (direction == -1)
			{
				player->frame = F_ANGLE_45;
			}
			else if (direction == 0)
			{
				//player->frame = F_DRIVE_1;
			}
		}

		player->trigger.shape->circle->center = player->frontWheel.circle->center;
	}
	if (gameWon)
	{
		player->frame = 28;
		winTimer -= milliseconds;
		if( winTimer < 0)
		{
			winTimer = winCooldown;
			gameWon = false;
			timeM = 0;
			timeS = 0;
			timeMS = 0;
			gameLoadTrackSelect();
		}
	}
	else
	{
		timeMS += milliseconds;
		if( timeMS >= 1000)
		{
			timeS++;
			timeMS = 0;
		}
		if( timeS == 60)
		{
			timeM++;
			timeS = 0;
		}

		playerVelocityX = player->frontWheel.velocity.x;

	}

	idleSfxTimer -= milliseconds;
	if( idleSfxTimer <= 0)
	{
		soundPlay(idleSfx);
		idleSfxTimer = idleSfxTimerCooldown;
	}

	if( player->state == STATE_DRIVING || player->state == STATE_WHEELIE ||
		player->state == STATE_LEFT || player->state == STATE_RIGHT)
	{
		driveSfxTimer -= milliseconds;
		if( driveSfxTimer <= 0)
		{
			soundPlay(driveSfx);
			driveSfxTimer = DriveSfxTimerCooldown;
		}
	}
	else
	{
		soundStop(driveSfx);
		driveSfxTimer = 0;
	}
}

static void playerHandleInput(Player* player, uint32_t milliseconds)
{
	static int overheatCooldown = 3000;
	float deltaTime = milliseconds / 1000.0f;
	if (inputKeyPressed('P'))
	{
		if (!pressedLastFrame)
		{
			debugToggle = !debugToggle;
			G_DEBUG = !G_DEBUG;
		}
		pressedLastFrame = true;
	}
	else
	{
		pressedLastFrame = false;
	}

	if ((player->isGrounded || player->onRamp) && player->state != STATE_OVERHEAT)
	{
		if (inputKeyPressed(INPUT_KEY_LOW_GEAR))
		{

			if (player->heat.current < player->heat.max / 2)
			{
				player->heat.current += player->heat.incRate * deltaTime;
			}
			else
			{
				player->heat.current -= player->heat.decRate * deltaTime;
			}

			float angleInRadians = player->angle * (float)(M_PI / 180.0f);

			// Calculate forward direction from angle
			float forwardX = (float)cos(angleInRadians);
			float forwardY = (float)sin(angleInRadians);

			float speed = (float)sqrt(player->frontWheel.velocity.x * player->frontWheel.velocity.x +
				player->frontWheel.velocity.y * player->frontWheel.velocity.y);
			if (speed < playerSpeed.low.current)
			{
				float velocityChange = playerSpeed.low.acceleration * deltaTime;
				//player->frontWheel.velocity.x += velocityChange;
				player->frontWheel.velocity.x += forwardX * velocityChange;
				player->frontWheel.velocity.y += forwardY * velocityChange;
			}

			if (speed > playerSpeed.low.current && !player->onRamp)
			{
				// Normalize the velocity vector and scale it to top speed
				float scaleFactor = playerSpeed.low.current / speed;
				player->frontWheel.velocity.x *= scaleFactor;
				player->frontWheel.velocity.y *= scaleFactor;
			}

			if (player->state != STATE_WHEELIE)
			{
				player->state = STATE_DRIVING;
			}
		}
		else if (inputKeyPressed(INPUT_KEY_HIGH_GEAR))
		{
			if (player->heat.current < player->heat.max)
			{
				player->heat.current += player->heat.incRate * deltaTime;
			}
			float angleInRadians = player->angle * (float)(M_PI / 180.0f);

			// Calculate forward direction from angle
			float forwardX = (float)cos(angleInRadians);
			float forwardY = (float)sin(angleInRadians);

			float speed = (float)sqrt(player->frontWheel.velocity.x * player->frontWheel.velocity.x +
				player->frontWheel.velocity.y * player->frontWheel.velocity.y);
			if (speed < playerSpeed.high.current)
			{
				float velocityChange = playerSpeed.high.acceleration * deltaTime;
				//player->frontWheel.velocity.x += velocityChange;
				player->frontWheel.velocity.x += forwardX * velocityChange;
				player->frontWheel.velocity.y += forwardY * velocityChange;
			}

			if (speed > playerSpeed.high.current && !player->onRamp)
			{
				// Normalize the velocity vector and scale it to top speed
				float scaleFactor = playerSpeed.high.current / speed;
				player->frontWheel.velocity.x *= scaleFactor;
				player->frontWheel.velocity.y *= scaleFactor;
			}

			if (player->state != STATE_WHEELIE)
			{
				player->state = STATE_DRIVING;
				if (player->heat.current > player->heat.max)
				{
					player->state = STATE_OVERHEAT;
					soundPlay(overheatSfx);
				}
			}
		}
		else
		{
			player->frontWheel.velocity.x -= playerBrakeForce * deltaTime;

			if (player->heat.current > 0)
			{
				player->heat.current -= player->heat.decRate * deltaTime;
				if (player->heat.current < 0)
				{
					player->heat.current = 0;
				}
			}
			//player->frontWheel.velocity.y -= 1000.0f * deltaTime;
			if (player->frontWheel.velocity.x < 0)
			{
				player->frontWheel.velocity.x = 0;
				if (player->state != STATE_OVERHEAT)
					player->state = STATE_IDLE;
				player->frame = F_IDLE;
			}
		}
	}
	else if (player->state == STATE_OVERHEAT)
	{
		if (player->frontWheel.velocity.x > 0)
		{
			player->frontWheel.velocity.x -= playerBrakeForce * deltaTime;
			if (player->frontWheel.velocity.x < 0)
				player->frontWheel.velocity.x = 0;

		}
		else
		{
			player->targetLane = 3;
			if (player->obj.position.y > lanes[player->targetLane] - 54)
			{
				player->frontWheel.circle->center.y -= player->obj.velocity.y * deltaTime;
			}
			else
			{
				player->currentLane = player->targetLane;
				
			}
		}
		player->frame = F_IDLE;
		overheatCooldown -= milliseconds;
		player->heat.current = (overheatCooldown * 100.0f) / 3000.0f;
		if (overheatCooldown <= 0)
		{
			player->state = STATE_IDLE;
			player->heat.current = 0;


			overheatCooldown = 3000;
		}
	}

	if (player->state != STATE_OVERHEAT)
	{
		if (player->frame != F_LEFT_TURN && player->frame != F_RIGHT_TURN)
		{
			//player->isGrounded = false;
		}

		if (fabs(player->obj.position.y - (float)lanes[player->currentLane]) < 5.0f)
		{
			if (inputKeyPressed(INPUT_KEY_TURN_LEFT) && playerVelocityX > 0 && player->targetLane == player->currentLane)
			{
				if (player->targetLane != laneCount - 1)
				{
					++player->targetLane;
					player->targetY = lanes[player->targetLane];
				}
			}
			else if (inputKeyPressed(INPUT_KEY_TURN_RIGHT) && playerVelocityX > 0 && player->targetLane == player->currentLane)
			{
				if (player->targetLane != 0)
				{
					--player->targetLane;
					player->targetY = lanes[player->targetLane];
				}
			}
			//player->isGrounded = true;
			player->angle = 0;
		}
	}



	if (player->nextWheelieFrameUpdate > milliseconds)
	{
		player->nextWheelieFrameUpdate -= milliseconds;
	}
	else if (player->isGrounded)
	{
		player->nextWheelieFrameUpdate = 50;
		if (inputKeyPressed(INPUT_KEY_TILT_LEFT) && (inputKeyPressed(INPUT_KEY_LOW_GEAR) || inputKeyPressed(INPUT_KEY_HIGH_GEAR))) // Only wheelie when moving
		{
			// Transition from shallow to steep angles
			switch (player->frame)
			{
			case F_ANGLE_15:
				player->frame = F_ANGLE_30;
				break;
			case F_ANGLE_30:
				player->frame = F_ANGLE_45;
				break;
			case F_ANGLE_45:
				player->frame = F_ANGLE_75;
				break;
			case F_ANGLE_75:
				player->frame = F_ANGLE_90;
				break;
			case F_ANGLE_90:
			//	player->frame = F_ANGLE_120;
			//	break;
			//case F_ANGLE_120:
				playerCrash(player);
				break;
			default:
				player->state = STATE_WHEELIE;
				player->frame = F_ANGLE_15;
				break;
			}
		}
		else // STOP WHEELIE
		{
			switch (player->frame)
			{
			case F_ANGLE_90:
				player->frame = F_ANGLE_75;
				break;
			case F_ANGLE_75:
				player->frame = F_ANGLE_45;
				break;
			case F_ANGLE_45:
				player->frame = F_ANGLE_30;
				break;
			case F_ANGLE_30:
				player->frame = F_ANGLE_15;
				break;
			case F_ANGLE_15:
				player->state = STATE_DRIVING;
				player->frame = F_DRIVE_1;
				break;
			}
		}



	}
	else if (!player->isGrounded && !player->onRamp)
	{
		player->nextWheelieFrameUpdate = 50;

		if (inputKeyPressed(INPUT_KEY_TILT_RIGHT))
		{
			switch (player->frame)
			{
			case F_ANGLE_90:
				player->frame = F_ANGLE_75;
				break;
			case F_ANGLE_75:
				player->frame = F_ANGLE_45;
				break;
			case F_ANGLE_45:
				player->frame = F_ANGLE_30;
				break;
			case F_ANGLE_30:
				player->frame = F_ANGLE_15;
				break;
			case F_ANGLE_15:
				player->frame = F_DRIVE_1;
				break;
			case F_DRIVE_1:
				player->frame = F_ANGLE_345;
				break;
			case F_ANGLE_345:
				player->frame = F_ANGLE_330;
				break;
			case F_ANGLE_330:
				player->frame = F_ANGLE_315;
				break;
			case F_ANGLE_315:
				player->frame = F_ANGLE_285;
				break;
			default:
				break;
			}
		}
		else if (inputKeyPressed(INPUT_KEY_TILT_LEFT))
		{
			switch (player->frame)
			{
			case F_ANGLE_285:
				player->frame = F_ANGLE_315;
				break;
			case F_ANGLE_315:
				player->frame = F_ANGLE_330;
				break;
			case F_ANGLE_330:
				player->frame = F_ANGLE_345;
				break;
			case F_ANGLE_345:
				player->frame = F_DRIVE_1;
				break;
			case F_DRIVE_1:
				player->frame = F_ANGLE_15;
				break;
			case F_ANGLE_15:
				player->frame = F_ANGLE_30;
				break;
			case F_ANGLE_30:
				player->frame = F_ANGLE_45;
				break;
			case F_ANGLE_45:
				player->frame = F_ANGLE_60;
				break;
			case F_ANGLE_60:
				player->frame = F_ANGLE_75;
				break;
				//case F_ANGLE_75:
				//	player->frame = F_ANGLE_90;
				//	break;
				//case F_ANGLE_90:
				//	player->frame = F_ANGLE_120;
				//	break;
				//case F_ANGLE_120:
				//	player->frame = F_ANGLE_180;
				//	break;
				//case F_ANGLE_180:
				//	player->frame = F_ANGLE_225;
				//	break;
				//case F_ANGLE_225:
				//	player->frame = F_ANGLE_270;
				//	break;
				//case F_ANGLE_270:
				//	player->frame = F_ANGLE_285;
				//	break;
			default:
				break;
			}
		}
	}

}

/// @brief Moves the player into their target lane and updates state to match
/// @param player Player object
/// @param milliseconds
void _moveToTargetLane(Player* player, uint32_t milliseconds)
{
	float deltaTime = milliseconds / 1000.0f;
	if (player->currentLane < player->targetLane)
	{
		if( player->state != STATE_WHEELIE )
		{
			player->state = STATE_LEFT;
			player->frame = F_LEFT_TURN;
		}
		player->obj.position.y -= player->obj.velocity.y * deltaTime;
		//player->frontWheel.circle->center.y -= player->obj.velocity.y * milliseconds;
		if (player->obj.position.y < lanes[player->targetLane])
		{
			player->currentLane = player->targetLane;
			player->obj.position.y = (float)lanes[player->currentLane];
		}
	}
	else if( player->currentLane > player->targetLane )
	{
		if (player->state != STATE_WHEELIE)
		{
			player->state = STATE_RIGHT;
			player->frame = F_RIGHT_TURN;
		}
		player->obj.position.y += player->obj.velocity.y * deltaTime;
		//player->frontWheel.circle->center.y += player->obj.velocity.y * milliseconds;

		if (player->obj.position.y > lanes[player->targetLane])
		{
			player->currentLane = player->targetLane;
			player->obj.position.y = (float)lanes[player->currentLane];
			Vector2 playerPosition = { player->obj.position.x, player->obj.position.y };
			player->frontWheel.circle->center = vector2Add(playerPosition, player->frontWheelOffset);
		}
	}
}

static void _playerOnCollision(Collision* collision, uint32_t milliseconds)
{

	Player* player = (Player*)collision->self->obj;

	player->frontWheel.circle->center.y -= collision->normal.y * collision->depth;

	//player->frontWheel.velocity.y += collision->normal.y * collision->depth * milliseconds / 1000;
	//player->frontWheel.velocity.x += collision->normal.x * collision->depth * milliseconds / 1000;
	if (player->frontWheel.velocity.y < 0)
	{
		player->frontWheel.velocity.y = 0;
	}
	if (player->frontWheel.velocity.x < 0)
	{
		player->frontWheel.velocity.x = 0;
	}
	if (collision->normal.x > 0) direction = -1;
	if (collision->normal.x < 0) direction = 1;
	if (collision->normal.x > -0.1f && collision->normal.x < 0.1f) direction = 0;

	float angle = 0;
	if (direction == -1)
	{
		
		angle = vector2Angle(collision->other->shape->trapezoid->a, collision->other->shape->trapezoid->b) * 180 / (float)M_PI;
		if( player->frame >= 15 && player->frame <= 18)
		{
			player->frontWheel.velocity.y = 0;
			playerCrash(player);
		}
	}
	else if (direction == 1)
	{
		angle = vector2Angle(collision->other->shape->trapezoid->c, collision->other->shape->trapezoid->d) * 180 / (float)M_PI;
	}
	else
	{
		angle = 0;
	}
	playerSetAngle(player, angle);

	player->onRamp = true;
}

static void _playerOnTrigger(Collision* collision, uint32_t milliseconds)
{
	Player* player = (Player*)collision->self->obj;

	if(objCompareTag(collision->other->obj, "_ramp"))
	{
		player->onRamp = true;
	}
	else
	{
		player->onRamp = false;
		direction = 0;
	}

}

static void playerSetAngle(Player* player, float angle)
{
	static float prevAngle = 0;
	if (angle == prevAngle) return;

	if (angle > 0)
	{
		// Convert angle to radians
		float angleInRadians = angle * ((float)M_PI / 180.0f);

		// Calculate current velocity magnitude to conserve energy
		float currentVelocity = sqrtf(
			player->frontWheel.velocity.x * player->frontWheel.velocity.x +
			player->frontWheel.velocity.y * player->frontWheel.velocity.y
		);

		// Redistribute velocity based on new angle
		player->frontWheel.velocity.x = currentVelocity * cosf(angleInRadians);
		player->frontWheel.velocity.y = currentVelocity * sinf(angleInRadians);
	}
	player->angle = angle;
	prevAngle = angle;
}

float playerGetVelocityX()
{
	return playerVelocityX;
}

Collider* _playerGetCollider(Object* obj)
{
	Player* player = (Player*)obj;
	return &player->collider;
}

Collider* playerGetTrigger(Object* obj)
{
	Player* player = (Player*)obj;
	return &player->trigger;
}

void playerSetTopSpeed(Player* player, float speed)
{
	playerSpeed.low.current = speed;
	playerSpeed.high.current = speed;
}

void playerResetTopSpeed(Player* player)
{
	playerSpeed.low.current = playerSpeed.low.max;
	playerSpeed.high.current = playerSpeed.high.max;
}

void playerCooldown(Player* player)
{
	player->heat.current = player->heat.current / 2;
}

bool playerIsWheelie(Player* player)
{
	if( player->state == STATE_WHEELIE)
	{
		return true;
	}
	return false;
}

void playerCrash(Player* player)
{
	player->state = STATE_ANIM_PLAY;
	player->crashWheelie.bPlaying = true;
	soundPlay(crashSfx);
}

void playerExitRamp(Player* player)
{

}

void _playerOnTriggerExit(Collision* collision, uint32_t milliseconds)
{
	Player* player = (Player*)collision->self->obj;
	player->onRamp = false;
	direction = 0;
}

uint8_t playerGetLaneIdx(Player* player)
{
	return player->currentLane;
}

void playerWin()
{
	gameWon = true;
	soundPlay(winSfx);
}


void drawTime(uint32_t m, uint32_t s, uint32_t ms)
{
	spriteDraw(numberSprite, (Coord2D) { 800, 955 }, 1, m, 1.0f, false);

	uint8_t sTens = (int)(s / 10);
	uint8_t sOnes = s % 10;

	spriteDraw(numberSprite, (Coord2D) { 870, 955 }, 1, sTens, 1.0f, false);
	spriteDraw(numberSprite, (Coord2D) { 910, 955 }, 1, sOnes, 1.0f, false);

	uint8_t msHundreds = (int)(ms / 10) % 10;
	uint8_t msThousands =(int)(ms / 100);
	spriteDraw(numberSprite, (Coord2D) { 970, 955 }, 1, msThousands, 1.0f, false);
	spriteDraw(numberSprite, (Coord2D) { 1010, 955 }, 1, msHundreds, 1.0f, false);
}