#pragma once
#include <string>
#include <glm\glm.hpp>
#include <CaveLib\Shader.h>

#include "CrimeScene.h"

class AssimpModel;
class ShaderProgram;
class Bbox;
class MapObject
{	
	AssimpModel* model;
	glm::vec3 position;
	glm::vec3 rotation;
	float scale;
	bool interactable;
	bool standardVisible;
	std::string description;

	glm::mat4 modelMatrix;

public:
	MapObject(AssimpModel* model, glm::vec3 position, glm::vec3 rotation, float scale, bool interactable, bool standardVisible, std::string description);
	~MapObject();

	Bbox getBoundingBox(glm::mat4* viewMatrix);
	glm::vec3 getPosition();
	void setPosition(glm::vec3 newPosition);
	glm::vec3 getRotation();
	void setRotation(glm::vec3 newRotation);
	float getScale();
	void setScale(float newScale);
	bool getIsInteractable();
	void setIsInteractable(bool newIsInteractable);
	bool getIsStandardVisible();
	void setIsStandardVisible(bool newIsStandardVisible);
	std::string getDescription();
	void setDescription(std::string newDescription);

	glm::mat4 getModelMatrix();
	void setModelMatrix(glm::mat4 newModelMatrix);

	void draw(Shader<CrimeScene::Uniforms>* shader);
	void drawBoundingBox(glm::mat4* viewMatrix);
};

