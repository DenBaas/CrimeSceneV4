#pragma once
#include <VrLib\Device.h>
#include <VrLib\Kernel.h>
#include <glm\glm.hpp>
#include <windows.h>
#include <VrLib/ClusterData.h>
#include <vector>
#include "SharedInfo.h"

#define MAXMOVEMENTSPEED 0.1f
#define MOVEMENTACCELERATION 90
#define ROTATIONSPEED glm::radians(3.0)

class Player
{
private:
	ClusterData<SharedInfo> sharedInfo;
	glm::mat4 position;
	float currentmovementspeed;
	enum Direction { forward, backwards, left, right } direction;

public:
	Player();
	~Player();
	glm::vec4 getPosition();
	void setPosition(glm::vec4 newPosition);
	float getRotation();
	void setRotation(float newRotation);
	void update(glm::mat4 hoofdmatrix);
	void update(const glm::vec2 &leftstick, const glm::vec2 &rightstick, float elapsedTime);

	glm::mat4 getPlayerMatrix();

	std::vector<glm::vec4> positions;
};
