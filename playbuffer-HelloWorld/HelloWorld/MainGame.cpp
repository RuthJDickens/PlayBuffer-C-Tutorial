#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include <iostream>

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

enum Agent8States
{
	STATE_APPEAR = 0,
	STATE_HALT,
	STATE_PLAY,
	STATE_DEAD,
};

struct GameState
{
	int score;
	Agent8States agentState = STATE_APPEAR;
};

GameState gameState;

enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_FAN,
	TYPE_TOOL,
	TYPE_COIN,
	TYPE_STAR,
	TYPE_LASER,
	TYPE_DESTROYED,
};

void HandlePlayerControls();
void UpdateFan();
void UpdateTools();
void UpdateCoinsAndStars();
void UpdateLasers();
void UpdateDestroyed();
void UpdateAgent8();

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::StartAudioLoop("music");

	//GameObjects are structs declared in Play.h
	//CreateGameObject creates a GameObject instance with type, position, collision radius and spriteId 
	//spriteId is found via PlayGraphics with the sprite name given
	//CreateGameObject returns an int type - GetGameObject takes an int type argument

	Play::CreateGameObject(TYPE_AGENT8, { 115, 0 }, 50, "agent8");
	int id_fan = Play::CreateGameObject(TYPE_FAN, { 1140,217 }, 0, "fan");
	Play::GetGameObject(id_fan).velocity = { 0, -3 };
	Play::GetGameObject(id_fan).animSpeed = 1.0f;

}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	//Draw functions find relevant image and make it visible by adding it to the drawing buffer
	Play::DrawBackground();

	UpdateAgent8();
	UpdateFan();
	UpdateTools();
	UpdateCoinsAndStars();
	UpdateLasers();
	UpdateDestroyed();
	Play::DrawFontText("64px", "Arrow Keys to move Vertical, Space to Shoot", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 }, Play::CENTRE);	//Uses the font sprite in the sprite folder with the name including "64px" to display the text in "", the Display width and height are referred to to position it at the centre bottom, the text is written with a centre allignment.
	Play::DrawFontText("132px", "SCORE = " + std::to_string(gameState.score), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);		//Similar to above but with different font (larger) and displayed at the top centre. This is called every frame so the score display will change as the score changes. *Not sure about the height.

	Play::PresentDrawingBuffer();	//Takes everything loaded in a presents it so the player can see it.

	//MainGameUpdate must return a bool variable so it checks if the escape key has been pressed
	//if true the game closes due to code in the Play.h file.
	return Play::KeyDown( VK_ESCAPE );
}

void HandlePlayerControls()
{
	//Creates a reference to the agent8 game object to directly alter its variables
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	//Movement
	if (Play::KeyDown(VK_UP))
	{
		obj_agent8.velocity = { 0, -4 };
		Play::SetSprite(obj_agent8, "agent8_climb", 0.25f);
	}
	if (Play::KeyDown(VK_DOWN))
	{
		obj_agent8.acceleration = { 0, 1 };
		Play::SetSprite(obj_agent8, "agent8_fall", 0);
	}
	//When no buttons are pressed agent8 is brought to a halt with code that mimics gravity
	//Eact code that brings agent8 to a halt is dependent on velocity - STATE_HALT plays animation before allowing player control again
	else
	{
		if (obj_agent8.velocity.y > 5)
		{
			gameState.agentState = STATE_HALT;
			Play::SetSprite(obj_agent8, "agent8_halt", 0.333f);
			obj_agent8.acceleration = { 0,0 };

		}
		else
		{
			Play::SetSprite(obj_agent8, "agent8_hang", 0.02f);
			obj_agent8.velocity *= 0.5f;
			obj_agent8.acceleration = { 0,0 };
		}
		
	}
	//Play.h UpdateGameObject function implements all changes made by altering velocity, pos, frame etc.
	Play::UpdateGameObject(obj_agent8);

	//Shoot
	if (Play::KeyPressed(VK_SPACE))
	{
		//firePos set in relation to agent8's position so it appears to spawn from gun
		Vector2D firePos = obj_agent8.pos + Vector2D(155, -75);
		int id = Play::CreateGameObject(TYPE_LASER, firePos, 30, "laser");
		Play::GetGameObject(id).velocity = { 32,0 };
		Play::PlayAudio("shoot");
	}

}


void UpdateFan()
{
	GameObject& obj_fan = Play::GetGameObjectByType(TYPE_FAN);

	//Spawn tool - 1 in 100 chance of spawning per frame
	if (Play::RandomRoll(100) == 50)
	{
		int id = Play::CreateGameObject(TYPE_TOOL, obj_fan.pos, 50, "driver");
		GameObject& obj_tool = Play::GetGameObject(id);
		//Velocity on y axis randomly set to 6 (diagonally down), -6 (diagonally up) or 0 (straight on)
		obj_tool.velocity = Point2f(-8, Play::RandomRollRange(-1, 1) * 6);

		//50:50 chance to change spanner
		//Spanner has different appearance and behaviour to add variety to gameplay
		if (Play::RandomRoll(2) == 1)
		{
			Play::SetSprite(obj_tool, "spanner", 0);
			obj_tool.radius = 100;
			obj_tool.velocity.x = -4;
			obj_tool.rotSpeed = 0.1f;
		}

		Play::PlayAudio("tool");
	}

	//One in 150 chance of spawning a coin
	if (Play::RandomRoll(150) == 1)
	{
		int id = Play::CreateGameObject(TYPE_COIN, obj_fan.pos, 40, "coin");
		GameObject& obj_coin = Play::GetGameObject(id);
		obj_coin.velocity = { -3,0 };
		obj_coin.rotSpeed = 0.1f;
	}
	//implements any changes to fan variables that occurred this frame.
	Play::UpdateGameObject(obj_fan);

	//If the fan is leaving the defined display area it is set to move back in the opposite direction
	if (Play::IsLeavingDisplayArea(obj_fan))
	{
		obj_fan.pos = obj_fan.oldPos;
		obj_fan.velocity.y *= -1;
	}

	Play::DrawObject(obj_fan);
}

void UpdateTools()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	//Creates a vector of all the tool game objects currently active on screen to iterate through
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);
	for (int id : vTools)
	{
		GameObject& obj_tool = Play::GetGameObject(id);

		//If the tool hits the player and there state is not already set to dead
		if (gameState.agentState != STATE_DEAD && Play::IsColliding(obj_tool, obj_agent8))
		{
			Play::StopAudioLoop("music");
			Play::PlayAudio("die");
			gameState.agentState = STATE_DEAD;
		}

		Play::UpdateGameObject(obj_tool);

		//Tool bounces off the top or bottom by checking if it's leaving the display area only on the vertical axis and changing the y velocity
		if(Play::IsLeavingDisplayArea(obj_tool, Play::VERTICAL))
		{
			obj_tool.pos = obj_tool.oldPos;
			obj_tool.velocity.y *= -1;
		}

		Play::DrawObjectRotated(obj_tool);

		//When the tool goes past the left side of the screen it is no longer visible so is destroyed so it won't slow down the game
		if (!Play::IsVisible(obj_tool))
		{
			//Destroy game object takes an int type argument
			Play::DestroyGameObject(id);
		}
	}
}

void UpdateCoinsAndStars()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	//vector of coins
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);

	//Coin Behaviour
	for (int id_coin : vCoins)
	{
		GameObject& obj_coin = Play::GetGameObject(id_coin);
		//Creating a bool variable acts like a 'hit' state;
		//the coin isn't destroyed right away so particles can be spawned in relation to it,
		bool hasCollided = false;

		//Coin collides with player and spawns particle effects
		if (Play::IsColliding(obj_coin, obj_agent8))
		{
			//Particle Effects
			//for loop iteration simultaneously limits the no. of particles to four and defines the no. used to calculate rotation
			for (float rad{ 0.25f }; rad < 2.0f; rad += 0.5f)	//results in rad = 0.25f, 0.75f, 1.25f and 1.75f,
			{
				int id = Play::CreateGameObject(TYPE_STAR, obj_agent8.pos, 0, "star");
				GameObject& obj_star = Play::GetGameObject(id);

				//Combination of rotation speed and y axis acceleration creates unique particle animation
				obj_star.rotSpeed = 0.1f;
				obj_star.acceleration = { 0.0f, 0.5f };

				//The direction is determined by the rad multiplied by Pi,
				//Results in stars moving diagonally away from each other like four points of a square
				Play::SetGameObjectDirection(obj_star, 16, rad * PLAY_PI);
			}

			//bool set to true so coin can be destroyed later
			hasCollided = true;
			gameState.score += 500;
			Play::PlayAudio("collect");
		}

		Play::UpdateGameObject(obj_coin);
		Play::DrawObjectRotated(obj_coin);

		//Coin is destroyed when it has moved beyond visible display area OR it has collided with the player
		if (!Play::IsVisible(obj_coin) || hasCollided);
		{
			Play::DestroyGameObject(id_coin);
		}
	}

	//vector of stars created so they're movement can be updated every frame (wouldn't otherwise move)
	//they are also destroyed when outside the screen
	std::vector<int> vStars = Play::CollectGameObjectIDsByType(TYPE_STAR);
	for (int id_star : vStars)
	{
		GameObject& obj_star = Play::GetGameObject(id_star);
		Play::UpdateGameObject(obj_star);
		Play::DrawObjectRotated(obj_star);
		if (!Play::IsVisible(obj_star))
		{
			Play::DestroyGameObject(id_star);
		}
	}
}

void UpdateLasers()
{
	//Lasers need references to all things they can hit; tools and coins
	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(TYPE_LASER);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);

	for (int id_laser : vLasers)
	{
		//Each laser has its own bool variable similar to the coins
		GameObject& obj_laser = Play::GetGameObject(id_laser);
		bool hasCollided = false;

		//Each laser iterates through the tools vector to check if it has collided with any of the tools
		for (int id_tool : vTools)
		{
			GameObject& obj_tool = Play::GetGameObject(id_tool);
			if (Play::IsColliding(obj_laser, obj_tool))
			{
				//Laser bool variable set to true and tool type changed so each can be destroyed later
				hasCollided = true;
				obj_tool.type = TYPE_DESTROYED;
				gameState.score += 100;
			}
		}
		//Each laser then checks if it has collided with any of the coins
		for (int id_coin : vCoins)
		{
			GameObject& obj_coin = Play::GetGameObject(id_coin);
			if (Play::IsColliding(obj_laser, obj_coin))
			{
				//Same as when collided with tool but score change is different
				hasCollided = true;
				obj_coin.type = TYPE_DESTROYED;
				gameState.score -= 300;
			}
		}
		//Can't have a negative score
		if (gameState.score < 0)
		{
			gameState.score = 0;
		}

		Play::UpdateGameObject(obj_laser);
		Play::DrawObject(obj_laser);
		//Destroyed if it leaves visible area or has collided with something
		if (!Play::IsVisible(obj_laser) || hasCollided)
		{
			Play::DestroyGameObject(id_laser);
		}
	}
}

void UpdateDestroyed()
{
	//All objects which have collided with a laser and therefore set to destroyed has its own method so it can have specific behaviour before it is destroyed
	std::vector<int> vDead = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);

	for (int id_dead : vDead)
	{
		GameObject& obj_dead = Play::GetGameObject(id_dead);
		obj_dead.animSpeed = 0.2f;
		Play::UpdateGameObject(obj_dead);
		//0 is treated as false, 1 as true
		//By dividing by 2, the remainder will be 1 (true) every other frame
		if (obj_dead.frame % 2)
		{
			//Opacity changed every other frame for flashing animation
			//Increasingly transparent each frame so it gradually fades until it is destroyed
			Play::DrawObjectRotated(obj_dead, (10 - obj_dead.frame) / 10.0f);
		}
		//if object leaves screen or opacity reduced to 0 (because frame >=10), object destroyed.
		if (!Play::IsVisible || obj_dead.frame >= 10)
		{
			Play::DestroyGameObject(id_dead);
		}
	}
}

void UpdateAgent8()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	//Switch statement allows us to define our different states we will be switching between.
	switch (gameState.agentState)
	{
		//STATE_APPEAR starts the game and defines the intro sequence when starting the game
		case STATE_APPEAR:
			//Variables reset in case coming from the dead state
			obj_agent8.velocity = { 0,12 };
			obj_agent8.acceleration = { 0, 0.5f };
			Play::SetSprite(obj_agent8, "agent8_fall", 0);
			obj_agent8.rotation = 0;
			//Changes state when agent8 is a third of the way down the screen
			if (obj_agent8.pos.y >= DISPLAY_HEIGHT / 3)
			{
				gameState.agentState = STATE_PLAY;
			}
			break;

		case STATE_HALT:
			//HandlePlayerControls not called so player can't control agent8 while in this state
			//Velocity reduced every frame in state halt
			obj_agent8.velocity *= 0.9f;
			//State changes when animation is complete
			if (Play::IsAnimationComplete(obj_agent8))
			{
				gameState.agentState = STATE_PLAY;
			}
			break;

		//Gives player control of agent8
		case STATE_PLAY:
			HandlePlayerControls();
			break;

		case STATE_DEAD:
			//Death animation; spiralls off screen
			obj_agent8.acceleration = { -0.3f, 0.5f };
			obj_agent8.rotation += 0.25f;
			//Pressing space resets game
			if (Play::KeyPressed(VK_SPACE))
			{
				gameState.agentState = STATE_APPEAR;
				obj_agent8.pos = { 115, 0 };
				obj_agent8.velocity = { 0,0 };
				obj_agent8.frame = 0;
				Play::StartAudioLoop("music");
				gameState.score = 0;

				//Collects every remaining tool and destroyed via UpdateDestroyed
				for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_TOOL))
				{
					Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
				}
			}
			break;
	}

	Play::UpdateGameObject(obj_agent8);
	//If the player is leaving display area AND they are not dead
	if (Play::IsLeavingDisplayArea(obj_agent8) && gameState.agentState != STATE_DEAD)
	{
		//Stops player moving outside of play area
		obj_agent8.pos = obj_agent8.oldPos;
	}
	//Draws agent8's web from the middle top of his sprite
	Play::DrawLine({ obj_agent8.pos.x, 0 }, obj_agent8.pos, Play::cWhite);
	Play::DrawObjectRotated(obj_agent8);
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	//When the MainGameExit is called the manager closes.
	Play::DestroyManager();
	return PLAY_OK;
}

