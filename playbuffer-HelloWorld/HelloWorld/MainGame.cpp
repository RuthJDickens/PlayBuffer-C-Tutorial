#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include <iostream>
#include <vector>
#include <string>	//Importing library code, part of standard library namespace (namespace is an address) 

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;		//Declaring and initialising integers with global scope, to be used for creating the size of the playspace and also references specific locations in the playspace

enum Agent8States
{
	STATE_APPEAR = 0,
	STATE_HALT,
	STATE_PLAY,
	STATE_DEAD,		//An enumeration for the play states of the game, assigning numbers to each state starting with 0 so when I refer to these states the code will interpret it as a number.
};

struct GameState	//A struct is a variable which includes other variables (members), it keeps related information together and allows us to access these variables and change them by refering to the struct.
{
	int score;
	Agent8States agentState = STATE_APPEAR;		//agentState is a variable of type Agent8States - the enum we declared earlier, and is initialised as STATE_APPEAR so the game will start on this state.
};

GameState gameState;	//Global scope. After declaring the GameState struct, we create a variable of type GameState and give it a name; gameState to refer to it in the code

enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_FAN,
	TYPE_TOOL,
	TYPE_COIN,
	TYPE_STAR,
	TYPE_LASER,
	TYPE_DESTROYED,		//Another enumeration type variable, this time to include all the types for the game objects in the game. By assigning numbers to these types here when we refer to these types later the game will automatically assign that game object a number using this enum.
};

void HandlePlayerControls();
void UpdateFan();
void UpdateTools();
void UpdateCoinsAndStars();
void UpdateLasers();
void UpdateDestroyed();
void UpdateAgent8();	//All of the methods used in the code have to be declared first.

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )		//All code within is executed once when the game is first loaded
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );	//The play space dimensions declared earlier and used to create the play manager.
	Play::CentreAllSpriteOrigins();		//Centres the origins of all sprites so that they can be positioned accurately and all in relation to the same start point.
	Play::LoadBackground("Data\\Backgrounds\\background.png");		//Background image found through file path and loaded in.
	Play::StartAudioLoop("music");		//Audio played on a loop
	Play::CreateGameObject(TYPE_AGENT8, { 115, 0 }, 50, "agent8");		//Agent8 is created, assigned a type (0), given a location and a collision radius and the sprite is set to the first image in the folder named; "agent8".
	int id_fan = Play::CreateGameObject(TYPE_FAN, { 1140,217 }, 0, "fan");		//The fan is created in the same way as agent8 but given the name id_fan so it can be referred to in the next two lines. *I'm not sure why it is an integer type variable.
	Play::GetGameObject(id_fan).velocity = { 0, -3 };
	Play::GetGameObject(id_fan).animSpeed = 1.0f;		//Using the name id_fan assigned to the game object struct created before we access the velocity and anim speed to change them.

}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	Play::DrawBackground();		//Takes the background image loaded before and presents to the drawing buffer *I'm not sure what this is.

	UpdateAgent8();
	UpdateFan();
	UpdateTools();
	UpdateCoinsAndStars();
	UpdateLasers();
	UpdateDestroyed();		//Every frame each of the methods we've created gets called, making sure all and every change is caught though this can be inefficient and cause problems.
	Play::DrawFontText("64px", "Arrow Keys to move Vertical, Space to Shoot", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 }, Play::CENTRE);	//Uses the font sprite in the sprite folder with the name including "64px" to display the text in "", the Display width and height are referred to to position it at the centre bottom, the text is written with a centre allignment.
	Play::DrawFontText("132px", "SCORE = " + std::to_string(gameState.score), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);		//Similar to above but with different font (larger) and displayed at the top centre. This is called every frame so the score display will change as the score changes. *Not sure about the height.

	Play::PresentDrawingBuffer();	//Takes everything loaded in a presents it so the player can see it.
	return Play::KeyDown( VK_ESCAPE );	//MainGameUpdate must return a bool variable so it checks if the escape key has been pressed, if true the game closes due to code in the header file.
}

void HandlePlayerControls()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);	//Creates a reference to the game object present with type agent8 so it can be referred to in the code - there should be only one created

	//Movement
	if (Play::KeyDown(VK_UP))	//if the Up key is down this code is implemented
	{
		obj_agent8.velocity = { 0, -4 };	//the velocity is changed to -4 on the y axis causing the object to move upwards.
		Play::SetSprite(obj_agent8, "agent8_climb", 0.25f);		//the sprite of obj_agent8 is changed to the climb animation and the speed of animation is set.
	}
	if (Play::KeyDown(VK_DOWN))		//this code executes as long as the down key is pressed
	{
		obj_agent8.acceleration = { 0, 1 };		//the acceleration is set so the object moves down with an increasing velocity.
		Play::SetSprite(obj_agent8, "agent8_fall", 0);	//the sprite is changed again but doesn't need to animate
	}
	else  //if neither of the two above statements are true then this code executes.
	{
		if (obj_agent8.velocity.y > 5)		//nested if statement; if the velocity of agent8 on the y axis only is more than 5 then this implements
		{
			gameState.agentState = STATE_HALT;		//the game state is changed to HALT and the code in that state activates.
			Play::SetSprite(obj_agent8, "agent8_halt", 0.333f);		//The sprite and animation speed is changed.
			obj_agent8.acceleration = { 0,0 };		//The acceleration is set to zero so the velocity stops changing.

		}
		else   //if the above is not true then this code is implemented.
		{
			Play::SetSprite(obj_agent8, "agent8_hang", 0.02f);	//The sprite and anim speed are set.
			obj_agent8.velocity *= 0.5f;	//velocity is multiplied by 0.5 - it is halved.
			obj_agent8.acceleration = { 0,0 };	//The acceleration is set to 0 so the velocity stays the same.
		}	//This code is called every frame so together they bring agent8 to a stop but not immediately - it takes a few frames to bring agent8 to a complete stop.
		
	}
	Play::UpdateGameObject(obj_agent8);		//updates agent8 so any changes we've made to it's variables are implemented.

	//Shoot
	if (Play::KeyPressed(VK_SPACE))		//When the space is pressed - but not for as long as it is held down.
	{
		Vector2D firePos = obj_agent8.pos + Vector2D(155, -75);		//Creates a vector2D variable so that we can easily refer to the gun position when needed.
		int id = Play::CreateGameObject(TYPE_LASER, firePos, 30, "laser");		//Creates the laser game object at the firePos declared before, named id so we can refer to it.
		Play::GetGameObject(id).velocity = { 32,0 };	//Uses the name id to access and alter the laser's velocity on the x axis.
		Play::PlayAudio("shoot");	//Plays the shoot sound once.
	}

}


void UpdateFan()
{
	GameObject& obj_fan = Play::GetGameObjectByType(TYPE_FAN);	//a reference to the game object with type fan created earlier (the id_fan doesn't have scope here so we need a new way of referring to the fan)

	//Spawn tool
	if (Play::RandomRoll(100) == 50)	//There is a one in 100 chance of this code implementing
	{
		int id = Play::CreateGameObject(TYPE_TOOL, obj_fan.pos, 50, "driver");		//Creates a tool object named id using the driver sprite at the same position of the fan.
		GameObject& obj_tool = Play::GetGameObject(id);		//Creates a reference to the id game object called obj_tool
		obj_tool.velocity = Point2f(-8, Play::RandomRollRange(-1, 1) * 6);	//changes the velocity to -8 on the x axis and a randomly determined y value; -6,6 or 0 - diagonally up, down or straight on.

		//50:50 chance change sprite
		if (Play::RandomRoll(2) == 1)
		{
			Play::SetSprite(obj_tool, "spanner", 0);	//uses the reference created to change the sprite of the spawned tool
			obj_tool.radius = 100;
			obj_tool.velocity.x = -4;
			obj_tool.rotSpeed = 0.1f;		//along with the radius, x axis velocity and rotation speed (the spanner is larger, slower and rotates)
		}

		Play::PlayAudio("tool");		//Plays sound once
	}

	if (Play::RandomRoll(150) == 1)		//one in 150 chance of spawning coin
	{
		int id = Play::CreateGameObject(TYPE_COIN, obj_fan.pos, 40, "coin");	//creates coin game object at the fan called id.
		GameObject& obj_coin = Play::GetGameObject(id);		//creates reference to game object id.
		obj_coin.velocity = { -3,0 };
		obj_coin.rotSpeed = 0.1f;		//sets velocity and rotation speed of coin
	}

	Play::UpdateGameObject(obj_fan);	//implements any changes to fan variables that occurred this frame.

	if (Play::IsLeavingDisplayArea(obj_fan))	//if the fan is leaving the defined display area
	{
		obj_fan.pos = obj_fan.oldPos;	//the new position is set to the position of last frame
		obj_fan.velocity.y *= -1;		//the velocity is multiplies by -1 to make it go in the opposite direction
	}

	Play::DrawObject(obj_fan);		//presents fan so it is visible in play space
}

void UpdateTools()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);	//A vector is similar to an array in that it includes mutliple instances of a single type of variable - this time integers.
		//This vector includes the ids all off the tool game objects for reference in the later code.
	for (int id : vTools)	//for loop; for every id in the vector implement this code.
	{
		GameObject& obj_tool = Play::GetGameObject(id);

		//Hit Player
		if (gameState.agentState != STATE_DEAD && Play::IsColliding(obj_tool, obj_agent8))	//if the game state is NOT dead AND the tool and player are colliding
		{
			Play::StopAudioLoop("music");	//stop music playing
			Play::PlayAudio("die");		//Play sound once
			gameState.agentState = STATE_DEAD;	//change state to dead
		}

		Play::UpdateGameObject(obj_tool);

		//Bounce off of top and bottom
		if(Play::IsLeavingDisplayArea(obj_tool, Play::VERTICAL))	//If the tool is leaving the play space vertically
		{
			obj_tool.pos = obj_tool.oldPos;
			obj_tool.velocity.y *= -1;	//same as fan movement
		}

		Play::DrawObjectRotated(obj_tool);

		//Past left of screen
		if (!Play::IsVisible(obj_tool))		//if the tool is no longer visible (left the side of the screen)
		{
			Play::DestroyGameObject(id);	//destroy the object - the game object reference doesn't have scope in this part so we refer to the id.
		}
	}
}

void UpdateCoinsAndStars()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN); //Create a vector similar to the one for tools containing ids of all the coin objects created.

	//Coin Behaviour
	for (int id_coin : vCoins) //for every id in the vector
	{
		GameObject& obj_coin = Play::GetGameObject(id_coin);	//game object reference
		bool hasCollided = false; //creating a bool varaible and initialising it as false.

		//Hit Player
		if (Play::IsColliding(obj_coin, obj_agent8))	//when the coin and agent8 objects' collision radius overlaps
		{
			//Particle Effects
			for (float rad{ 0.25f }; rad < 2.0f; rad += 0.5f)	//starting with rad = 0.25, implement code for every rad that is less than 2, increasing rad by 0.5 ever time - results in 0.25f, 0.75f, 1.25f and 1.75f,
			{
				int id = Play::CreateGameObject(TYPE_STAR, obj_agent8.pos, 0, "star");	//Create star object, no collision radius
				GameObject& obj_star = Play::GetGameObject(id);	
				obj_star.rotSpeed = 0.1f;	//change rotation speed
				obj_star.acceleration = { 0.0f, 0.5f };		//change acceleration on the y
				Play::SetGameObjectDirection(obj_star, 16, rad * PLAY_PI);		//The direction is determined by the rad multiplied by Pi, each star has a different direction so they move diagonally away from each to other.
			}

			hasCollided = true;		//After the particle effects the collided bool is set to true
			gameState.score += 500;		//the score is updated
			Play::PlayAudio("collect");		//sound is played
		}

		Play::UpdateGameObject(obj_coin);
		Play::DrawObjectRotated(obj_coin);		//coin is updated and drawn for the player to see

		if (!Play::IsVisible(obj_coin) || hasCollided)		//is the coin is NOT visible (past the left of the screen) OR hasCollided is true.
		{
			Play::DestroyGameObject(id_coin);	//the object is destroyed
		}
	}

	std::vector<int> vStars = Play::CollectGameObjectIDsByType(TYPE_STAR);	//vector of stars created

	for (int id_star : vStars)	//for every star in the vector
	{
		GameObject& obj_star = Play::GetGameObject(id_star);
		Play::UpdateGameObject(obj_star);
		Play::DrawObjectRotated(obj_star);		//UpdateCoinsandStars is called every frame so every frame the objects are updated and drawn on screen

		if (!Play::IsVisible(obj_star))		//When they've left the play space and are no longer visible they are destroyed
		{
			Play::DestroyGameObject(id_star);
		}
	}
}

void UpdateLasers()
{
	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(TYPE_LASER);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);		//vectors made for lasers, tools and coins.

	for (int id_laser : vLasers)
	{
		GameObject& obj_laser = Play::GetGameObject(id_laser);
		bool hasCollided = false;		//Each laser object has a collision bool variable initally set to false

		for (int id_tool : vTools)
		{
			GameObject& obj_tool = Play::GetGameObject(id_tool);
			if (Play::IsColliding(obj_laser, obj_tool))		//Each tool checks if they are overlapping with a laser
			{
				hasCollided = true;		//This is a nested if statement so the hasCollided bool has scope and can be referenced
				obj_tool.type = TYPE_DESTROYED;		//The tool changes type so it will be updated with the UpdateDestroyed method
				gameState.score += 100;		//The score updates
			}
		}

		for (int id_coin : vCoins)
		{
			GameObject& obj_coin = Play::GetGameObject(id_coin);
			if (Play::IsColliding(obj_laser, obj_coin))
			{
				hasCollided = true;
				obj_coin.type = TYPE_DESTROYED;
				gameState.score -= 300;		//Same as tool but the score changes by different amount.
			}
		}

		if (gameState.score < 0)
		{
			gameState.score = 0;	//The score is set so it can't go below 0.
		}

		Play::UpdateGameObject(obj_laser);
		Play::DrawObject(obj_laser);		//laser updated and drawn for player

		if (!Play::IsVisible(obj_laser) || hasCollided)	//if the laser leaves the screen OR hasCollided is true
		{
			Play::DestroyGameObject(id_laser);	//the object is destroyed.
		}
	}
}

void UpdateDestroyed()
{
	std::vector<int> vDead = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);	//creates vector of game objects type destroyed

	for (int id_dead : vDead)
	{
		GameObject& obj_dead = Play::GetGameObject(id_dead);
		obj_dead.animSpeed = 0.2f;		//Change object animation speed
		Play::UpdateGameObject(obj_dead);	//Update the object

		if (obj_dead.frame % 2)		//Divides the objects current frame by 2 and takes the remainder - will be either 0 or 1, considered false and true by the code respectively. This is expression = 1 when the frame is an odd number so will be true every other frame.
		{
			Play::DrawObjectRotated(obj_dead, (10 - obj_dead.frame) / 10.0f);	//changes the opacity of the object relative to the frame it's on; the higher the frame the more transparent it is. Opacity must be a float between 0-1.
		}

		if (!Play::IsVisible || obj_dead.frame >= 10)	//if object leaves screen or opacity reduced to 0 (because frame >=10), object destroyed.
		{
			Play::DestroyGameObject(id_dead);
		}
	}
}

void UpdateAgent8()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	switch (gameState.agentState)		//Switch statement allows us to define our different states we will be switching between.
	{
		case STATE_APPEAR:		//When gameState.agentState = STATE_APPEAR this will happen.
			obj_agent8.velocity = { 0,12 };
			obj_agent8.acceleration = { 0, 0.5f };		//Agent's velocity and acceleration are set.
			Play::SetSprite(obj_agent8, "agent8_fall", 0);	//Sprite is set and doesn't need to animate.
			obj_agent8.rotation = 0;	//rotation set to zero
			if (obj_agent8.pos.y >= DISPLAY_HEIGHT / 3)		//When agent8 has moved down a third of the display height the state changes (y starts at 0).
			{
				gameState.agentState = STATE_PLAY;
			}
			break;

		case STATE_HALT:
			obj_agent8.velocity *= 0.9f;	//multiplies the velocity by 0.9 every frame the state = halt (UpdateAgent8 called every frame).
			if (Play::IsAnimationComplete(obj_agent8))		//Only changes state when animation finished.
			{
				gameState.agentState = STATE_PLAY;
			}
			break;

		case STATE_PLAY:
			HandlePlayerControls();		//Gives player control of agent8
			break;

		case STATE_DEAD:
			obj_agent8.acceleration = { -0.3f, 0.5f };
			obj_agent8.rotation += 0.25f;		//When agent8 dies by colliding with tool, his acceleration and rotation are changed so he falls down in a small spiral to the left
			if (Play::KeyPressed(VK_SPACE))		//When the spacebar is pressed *I don't know why == true is specified.
			{
				gameState.agentState = STATE_APPEAR;	//Resets the game state to appear.
				obj_agent8.pos = { 115, 0 };
				obj_agent8.velocity = { 0,0 };
				obj_agent8.frame = 0;		//resets agent's position, velocity and frame
				Play::StartAudioLoop("music");		//Starts music again.
				gameState.score = 0;		//resets score

				for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_TOOL))
				{
					Play::GetGameObject(id_obj).type = TYPE_DESTROYED;		//Gets all left over tool objects still on screen and sets them to destroyed type so they will be destroyed in the same way every other tool/coin is.
				}
			}
			break;
	}

	Play::UpdateGameObject(obj_agent8);

	if (Play::IsLeavingDisplayArea(obj_agent8) && gameState.agentState != STATE_DEAD)	//If the player is leaving display area AND they are not dead
	{
		obj_agent8.pos = obj_agent8.oldPos;		//position set to position of last frame to stop him going any further
	}

	Play::DrawLine({ obj_agent8.pos.x, 0 }, obj_agent8.pos, Play::cWhite);	//Draws his web as a line starting from his x position and ending at his x and y position so it comes out of the middle top of his sprite.
	Play::DrawObjectRotated(obj_agent8);
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();		//When the MainGameExit is called the manager closes.
	return PLAY_OK;
}

