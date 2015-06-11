#include "Player.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <VrLib\Viewport.h>
#include <GL\GL.h>
#include <String>
#include <glm\gtx\vector_angle.hpp>
#include <iostream>
using namespace std;


/*
Entrypoint of the Player class
Author: Tycho krielaart - 20-5-2014
Last edit: <name> - dd-mm-yyyy
*/
Player::Player()
{
	ShowCursor(FALSE);
	sharedInfo->rotation = 0;
	sharedInfo.init();

}

Player::~Player()
{
	positions.clear();
}

/*
returns the position of the player
Author: Tycho krielaart - 20-5-2014
Last edit: <name> - dd-mm-yyyy
*/
glm::vec4 Player::getPosition()
{
	return sharedInfo->position;
}

/*
Set the position of the player
Author: Bas Rops - 27-05-2014
Last edit: <name> - dd-mm-yyyy
*/
void Player::setPosition(glm::vec4 newPosition)
{
	sharedInfo->position = newPosition;
}

/*
returns the angle of the player
Author: Tycho krielaart - 20-5-2014
Last edit: <name> - dd-mm-yyyy
*/
float Player::getRotation()
{
	return sharedInfo->rotation;
}

/*
Set the y-axis rotation of the player
Author: Bas Rops - 27-05-2014
Last edit: <name> - dd-mm-yyyy
*/
void Player::setRotation(float newRotation)
{
	sharedInfo->rotation = newRotation;
}

/*
returns the current matrix of the Player
Author: Tycho krielaart - 20-5-2014
Last edit: <name> - dd-mm-yyyy
*/
glm::mat4 Player::getPlayerMatrix()
{
	return position;
}



/*
call this once per frame.
this will update the player with the changes to the mouse
Author: Tycho krielaart - 20-5-2014
Last edit: <name> - dd-mm-yyyy
*/
void Player::update(glm::mat4 hoofdmatrix)
{
	POINT p;
	DWORD eigenId = GetCurrentThreadId();
	DWORD ActiveId = GetWindowThreadProcessId(GetForegroundWindow(), NULL);


	if (GetCursorPos(&p) && eigenId == ActiveId)
	{
		RECT r;
		GetWindowRect(GetActiveWindow(), &r);
		int xVerschill = (r.left + (Kernel::getInstance()->getWindowWidth() / 2)) - p.x;
		int yVerschill = (r.top + (Kernel::getInstance()->getWindowHeight() / 2)) - p.y;

		int xVerschilPos = abs(xVerschill);
		int yVerschilPos = abs(yVerschill);

		if (xVerschilPos < yVerschilPos)
		{
			if (currentmovementspeed <= MAXMOVEMENTSPEED)
				currentmovementspeed += MAXMOVEMENTSPEED / MOVEMENTACCELERATION;
		}
		else
		{
			if (xVerschill > 0)
			{
				sharedInfo->rotation -= ROTATIONSPEED;
				currentmovementspeed = 0;
				direction = left;
			}
			if (xVerschill < 0)
			{
				currentmovementspeed = 0;
				sharedInfo->rotation += ROTATIONSPEED;
				direction = right;
			}
		}

		glm::mat4 rotatie;
		//rotatie = glm::translate(hoofdmatrix, glm::vec3(sharedInfo->position));
		rotatie = glm::rotate(hoofdmatrix, sharedInfo->rotation, glm::vec3(0, 1, 0));	// <--- To Fix
		//rotatie = glm::translate(hoofdmatrix, -glm::vec3(sharedInfo->position));

		glm::vec4 verandering = (hoofdmatrix * glm::vec4(0, 0, 0, 1)) - (hoofdmatrix* glm::vec4(0, 0, currentmovementspeed, 1));
		verandering = verandering* rotatie;
		verandering.y = 0;
		if (yVerschill <0)
		{
			if (direction == backwards)
			{
				sharedInfo->position += verandering;
				direction = backwards;
			}
			else
			{
				currentmovementspeed = 0;
				direction = backwards;
			}
		}
		if (yVerschill >0)
		{
			if (direction == forward)
			{
				sharedInfo->position -= verandering;
				direction = forward;
			}
			else
			{
				currentmovementspeed = 0;
				direction = forward;
			}
		}

		SetCursorPos(r.left +(Kernel::getInstance()->getWindowWidth() / 2), r.top+ (Kernel::getInstance()->getWindowHeight() / 2));

		glm::mat4 nieuw;
		position = nieuw;

		//position = glm::translate(position, glm::vec3(sharedInfo->position));
		position = glm::rotate(position, sharedInfo->rotation, glm::vec3(0, 1, 0));
		//position = glm::translate(position, -glm::vec3(sharedInfo->position));

		position = glm::translate(position, glm::vec3(sharedInfo->position));
	}
}


/*
call this once per frame.
this will update the player using the hydra's 2 joysticks
Author: Johan Talboom
Last edit: <name> - dd-mm-yyyy
*/
void Player::update(const glm::vec2 &leftstick, const glm::vec2 &rightstick, float elapsedTime)
{

	if (glm::length(leftstick) < 2 && glm::length(rightstick) < 2)
	{
		glm::mat4 transform = glm::rotate(glm::mat4(), -sharedInfo->rotation, glm::vec3(0, 1, 0));
		sharedInfo->position += transform * glm::vec4(-leftstick.x, 0, leftstick.y, 0) * elapsedTime / 1000.0f;

		sharedInfo->rotation += rightstick.x * elapsedTime / 1000.0f;
	}


	position = glm::mat4();
	position = glm::rotate(position, sharedInfo->rotation, glm::vec3(0, 1, 0));
	position = glm::translate(position, glm::vec3(sharedInfo->position));
}
