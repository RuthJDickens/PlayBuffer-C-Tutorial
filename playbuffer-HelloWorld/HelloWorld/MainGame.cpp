#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

enum Types
{
	TYPE_NULL = -1,
	TYPE_ASTEROID,
	TYPE_AGENT8,
	TYPE_ROCK,
};

void UpdateRock();
void WrapMovement(GameObject& object);

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::CentreAllSpriteOrigins();
	static int x = 1;
	static int y = 2;
	static int z = 3;
	int id_rock = Play::CreateGameObject(TYPE_ASTEROID, { 853, DISPLAY_HEIGHT/2 }, 75, "agent8");
	int id_rock1 = Play::CreateGameObject(TYPE_AGENT8, { 640, DISPLAY_HEIGHT / 2 }, 75, "agent8");
	int id_rock2 = Play::CreateGameObject(TYPE_ROCK, { 426, DISPLAY_HEIGHT / 2 }, 75, "agent8");
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	//Play::ClearDrawingBuffer(Play::cBlack);
	//UpdateRock();
	GameObject& obj1 = Play::GetGameObjectByType(TYPE_ASTEROID);
	GameObject& obj2 = Play::GetGameObjectByType(TYPE_AGENT8);
	GameObject& obj3 = Play::GetGameObjectByType(TYPE_ROCK);
	static int rot1 = 1;
	static int rot2 = 2;
	static int rot3 = 3;
	obj1.rotation = rot1;
	Play::DrawObjectRotated(obj1, rot1);
	obj2.rotation = rot2;
	Play::DrawObjectRotated(obj2, rot1);
	obj3.rotation = rot3;
	Play::DrawObjectRotated(obj3, rot1);

	Play::PresentDrawingBuffer();
	
	return Play::KeyDown(VK_ESCAPE);
}

void UpdateRock()
{
	GameObject& obj_rock = Play::GetGameObjectByType(TYPE_ASTEROID);
	obj_rock.rotation = 0;
	//Play::SetGameObjectDirection(obj_rock, 4, obj_rock.rotation);
	Play::DrawObjectRotated(obj_rock);
	/*if (Play::IsLeavingDisplayArea(obj_rock))
	{
		WrapMovement(obj_rock);
	}*/
	Play::UpdateGameObject(obj_rock, true, 150);

}

void WrapMovement(GameObject& object)
{
	if (object.pos.x > DISPLAY_WIDTH + 100)
	{
		object.pos.x -= object.oldPos.x;
	}
	if (object.pos.y > DISPLAY_HEIGHT + 100)
	{
		object.pos.y -= object.oldPos.y;
	}
	if (object.pos.x < -100)
	{
		object.pos.x += object.oldPos.x * -1 + DISPLAY_WIDTH;
	}
	if (object.pos.y < -100)
	{
		object.pos.y += object.oldPos.y * -1 + DISPLAY_HEIGHT;
	}
}

// Gets called once when the player quits the game 
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}

