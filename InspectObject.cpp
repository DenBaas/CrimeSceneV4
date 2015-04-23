#include "InspectObject.h"
#include "MapObject.h"
#include "Player.h"

#include <Cavelib/cavelib.h>
#include <CaveLib/bbox.h>
#include <VrLib\Device.h>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/*
Constructor for object inspection
Author: Tycho krielaart - 23-5-2014
Last edit: Bas Rops - 11-06-2014
*/
InspectObject::InspectObject(MapObject* object, glm::vec4 position)
{
	this->originalModelMatrix = object->getModelMatrix();
	this->object = object;
	this->position = position;
}

/*
Destructor for the inspectObject
Reset the inspected object modelMatrix to its original matrix
Author: Tycho krielaart - 23-5-2014
Last edit: Bas Rops - 11-06-2014
*/
InspectObject::~InspectObject()
{
	object->setModelMatrix(originalModelMatrix);
}

/*
Returns the inspected item
Author: Tycho krielaart - 23-5-2014
Last edit: Bas Rops - 11-06-2014
*/
MapObject* InspectObject::getInspectedObject()
{
	return object;
}

/*
Updates the object inspection with its given arguments
Call once per frame, when an object is being inspected
Author: Tycho krielaart - 23-5-2014
Last edit: Bas Rops - 11-06-2014
*/
void InspectObject::updateView(glm::mat4 angleMatrix, bool zoomIn, bool zoomOut, Player* player, PositionalDevice* head)
{
	inspectMatrix = glm::mat4();
	
	//inspectMatrix = glm::rotate(inspectMatrix, 180.0f, glm::vec3(0, 1, 0));

	

	angles = angleMatrix;

	if (zoomIn)
	{
		glm::vec3 direction = glm::vec3(glm::cos(glm::radians(player->getRotation()) - 30.0f), 0.0f, glm::sin(glm::radians(player->getRotation()) - 30.0f)) * ROTATION_SPEED;
		zoom += direction;
	}
	if (zoomOut)
	{
		glm::vec3 direction = glm::vec3(glm::cos(glm::radians(player->getRotation()) - 30.0f), 0.0f, glm::sin(glm::radians(player->getRotation()) - 30.0f)) * ROTATION_SPEED;
		zoom -= direction;
	}


	inspectMatrix = glm::translate(inspectMatrix, -(glm::vec3(object->getBoundingBox(&player->getPlayerMatrix()).mMax.x , object->getBoundingBox(&player->getPlayerMatrix()).mMax.y , object->getBoundingBox(&player->getPlayerMatrix()).mMax.z ) ));
	inspectMatrix = glm::translate(inspectMatrix, -(glm::vec3(glm::cos(glm::radians(player->getRotation()) - 30.0f), 0.0f, glm::sin(glm::radians(player->getRotation()) - 30.0f) * 5)));
	inspectMatrix = glm::translate(inspectMatrix, -(glm::vec3(position) + zoom));
	inspectMatrix = glm::rotate(inspectMatrix, -90.0f, glm::vec3(0, 0, 1));
	inspectMatrix *= head->getData();
	inspectMatrix *= angles;
}

/*
Draw the object inspection
Call once per frame, when an object is being inspected
Author: Tycho krielaart - 23-5-2014
Last edit: Bas Rops - 11-06-2014
*/
void InspectObject::draw(Shader<CrimeScene::Uniforms>* shader)
{
	object->setModelMatrix(inspectMatrix);

	object->draw(shader);

	object->setModelMatrix(originalModelMatrix);
}