#include "MapObject.h"
#include "AssimpModel.h"

#include <CaveLib\Shader.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <json\reader.h>

using namespace std;
using namespace Json;

/*
Constructor
model = pointer to the object's model
position = position of the object
rotation = how the object is rotated on the x/y/z-axis
scale = scale of the object, 1.0f = not scaled up/down
interactable = if it is possible to interact with the object
standardVisible = if the object is visible without using a polylight
description = text that appears when the object is selected
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 09-05-2014
*/
MapObject::MapObject(AssimpModel* model, glm::vec3 position, glm::vec3 rotation, float scale, bool interactable, bool standardVisible, std::string description, float mass)
{
	this->model = model;
	this->position = position;
	this->rotation = rotation;
	this->scale = scale;
	this->interactable = interactable;
	this->standardVisible = standardVisible;
	this->description = description;

	this->modelMatrix = glm::mat4();
	
	//Add 90 to x-rotation and 180 to y-rotation to compensate for rotation in file,
	//because the mapeditor and simulator have different object rotations.
	this->modelMatrix = glm::translate(this->modelMatrix, this->position);
	this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(180.0f), glm::vec3(0, 1, 0));
	this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(this->rotation.z), glm::vec3(0, 0, 1));
	this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(this->rotation.x), glm::vec3(1, 0, 0));
	this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(-this->rotation.y), glm::vec3(0, 1, 0));
}

void MapObject::setPhysicsObject(string fileName, btVector3& rotation, btVector3& position){
	Json::Reader reader;
	Json::Value root;

	//Read the file
	std::string data = "";
	std::string line;
	std::ifstream pFile(fileName);
	if (pFile)
	{
		while (!pFile.eof() && pFile.good())
		{
			std::getline(pFile, line);
			data += line + "\n";
		}
	}
	else
	{
		return;
	}
	//Parse the data as JSON
	bool parsingSuccessful = reader.parse(data, root);
	if (!parsingSuccessful)
	{
		return;
	}
	float y = root["Presentatie4"]["hoogte"].asFloat();
	float x = root["Presentatie4"]["breedte"].asFloat();
	float z = root["Presentatie4"]["diepte"].asFloat();
	float mass = root["Presentatie4"]["mass"].asFloat();
	dimensionToTranslate = glm::vec3(root["Presentatie4"]["offsetX"].asFloat(), root["Presentatie4"]["offsetY"].asFloat(), root["Presentatie4"]["offsetZ"].asFloat());//
	
	btCollisionShape* colShape = new btBoxShape(btVector3(x,y,z));
	btTransform startTransform;
	startTransform.setIdentity();
	btVector3 origin(this->position.x, this->position.y, this->position.z);
	//rotation is fucked :)   
	btQuaternion newrotation;
	btVector3 rotation2 = btVector3(90, 180, 0);
	rotation += rotation2;
	newrotation.setEuler(glm::radians(rotation.y()), 
		glm::radians(rotation.z()),
		glm::radians(rotation.x()));
	startTransform.setOrigin(origin);
	startTransform.setRotation(newrotation);

	btDefaultMotionState* colMotionState = new btDefaultMotionState();
	btVector3 fallInertia;
	colShape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, colMotionState, colShape, fallInertia);
	BoundingBoxPhys = new btRigidBody(cInfo);
	BoundingBoxPhys->setCenterOfMassTransform(startTransform);
	BoundingBoxPhys->setGravity(btVector3(0, -9.8, 0));
	BoundingBoxPhys->setRestitution(0.0f);
	BoundingBoxPhys->setFriction(0.0f);
}

/*
Destructor
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 13-06-2014
*/
MapObject::~MapObject()
{

}

/*
Retrieve the position (glm::vec3) of the object
x = position on x-axis
y = position on y-axis
z = position on z-axis
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 06-05-2014
*/
glm::vec3 MapObject::getPosition()
{
	return this->position;
}

/*
Set the position of the object
newPosition = glm::vec3 containing the new position of the object
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 06-05-2014
*/
void MapObject::setPosition(glm::vec3 newPosition)
{
	this->position = newPosition;

	//this->modelMatrix = glm::translate(this->modelMatrix, this->position);
	////Add 90 to x-rotation and 180 to y-rotation to compensate for rotation in file,
	////because the mapeditor and simulator have different object rotations.
	//this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	//this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(180.0f), glm::vec3(0, 1, 0));
}

glm::vec3 MapObject::getPhysicsObjectPosition()
{
	btTransform trans;
	BoundingBoxPhys->getMotionState()->getWorldTransform(trans);
	glm::vec3 trans3 = { (float)trans.getOrigin().getX(), ((float)trans.getOrigin().getY()), (float)trans.getOrigin().getZ() };

	return trans3;
}

/*
Retrieve the rotation (glm::vec3) of the object
x = rotation around x-axis
y = rotation around y-axis
z = rotation around z-axis
Author: Bas Rops - 06-05-2014
Last edit: <name> - dd-mm-yyyy
*/
glm::vec3 MapObject::getRotation()
{
	return this->rotation;
}

/*
Set the rotation of the object
newRotation = glm::vec3 containing the new rotation of the object (x-axis, y-axis, z-axis)
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 06-05-2014
*/
void MapObject::setRotation(glm::vec3 newRotation)
{
	this->rotation = newRotation;
}

/*
Retrieve the scale of the object as a float
a scale of 0.5f means half the original size
a scale of 2.0f means twice the original size
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 06-05-2014
*/
float MapObject::getScale()
{
	return this->scale;
}

/*
Set the scale of the object
newScale = float containing the new scale of the object
a scale of 0.5f means half the original size
a scale of 2.0f means twice the original size
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 06-05-2014
*/
void MapObject::setScale(float newScale)
{
	this->scale = newScale;
}

/*
Check if the object is interactable or not
True if interactable, false if not
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 06-05-2014
*/
bool MapObject::getIsInteractable()
{
	return this->interactable;
}

/*
Set if the object is interactable or not
newIsInteractable = bool containing if the object is interactable or not
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 06-05-2014
*/
void MapObject::setIsInteractable(bool newIsInteractable)
{
	this->interactable = newIsInteractable;
}

/*
Check the standard visibility of the object
True if visible, false if not
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 06-05-2014
*/
bool MapObject::getIsStandardVisible()
{
	return this->standardVisible;
}

/*
Set the standard visibility of the object
newIsStandardVisible = bool containing the new standard visibility of the object
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 06-05-2014
*/
void MapObject::setIsStandardVisible(bool newIsStandardVisible)
{
	this->standardVisible = newIsStandardVisible;
}

/*
Retrieve the description of this object
Author: Bas Rops - 09-06-2014
Last edit: <name> - dd-mm-yyyy
*/
std::string MapObject::getDescription()
{
	return this->description;
}

/*
Set the description of this object
Author: Bas Rops - 09-06-2014
Last edit: <name> - dd-mm-yyyy
*/
void MapObject::setDescription(std::string newDescription)
{
	this->description = newDescription;
}

/*
Retrieve a glm::mat4 containing the model matrix of this object
Contains model position and model rotations around x- y- and z-axis
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 16-05-2014
*/
glm::mat4 MapObject::getModelMatrix()
{
	return this->modelMatrix;
}

/*
Set the modelMatrix of this object that contains model position and model rotations around x- y- and z-axis
newModelMatrix = the new modelMatrix of this object
Author: Bas Rops - 16-05-2014
Last edit: <name> - dd-mm-yyyy
*/
void MapObject::setModelMatrix(glm::mat4 newModelMatrix)
{
	this->modelMatrix = newModelMatrix;
}

/*
Draws the model of the object
shader = shader to draw the model with
Remember to set model-specific uniforms in this method!
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 23-05-2014
*/
void MapObject::draw(Shader<CrimeScene::Uniforms>* shader)
{
	if (this->model != nullptr)
	{
		if (shader)
		{
			//shader->setUniform(CrimeScene::Uniforms::scale, this->scale); //TODO?
			shader->setUniform(CrimeScene::Uniforms::objectVisible, this->standardVisible||true);
		}
		glm::mat4 newMat(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
		if (BoundingBoxPhys){
			btScalar transform[16];
			BoundingBoxPhys->getWorldTransform().getOpenGLMatrix(transform);
			float dArray[16] = { 0.0 };
			const float *pSource = (const float*) glm::value_ptr(modelMatrix);
			for (int i = 0; i < 16; ++i)
				dArray[i] = pSource[i];
			glm::vec3 boundingsvec = model->getBoundingBox().mMax - model->getBoundingBox().mMin;
			newMat = glm::mat4(pSource[0], pSource[1], pSource[2], pSource[3],
				pSource[4], pSource[5], pSource[6], pSource[7],
				pSource[8], pSource[9], pSource[10], pSource[11],
				transform[12], transform[13], transform[14], pSource[15]);
			newMat = glm::mat4(transform[0], transform[1], transform[2], transform[3], 
				transform[4], transform[5], transform[6], transform[7],
				transform[8], transform[9], transform[10], transform[11],
				transform[12], transform[13], transform[14], transform[15]);
			btQuaternion rot = BoundingBoxPhys->getWorldTransform().getRotation();
			btVector3 axis = rot.getAxis();
			//newMat = glm::rotate(newMat,rot.getAngle(), glm::vec3(axis.x(), axis.y(), axis.z()));
			newMat = glm::translate(newMat, dimensionToTranslate);
			model->draw(shader, &newMat);
		}
		else
			model->draw(shader, &modelMatrix);
	}
}

/*
Draws the bounding box of the object using old and ugly OpenGL, without shaders
Author: Bas Rops - 20-05-2014
Last edit: Bas Rops - 06-06-2014
*/
void MapObject::drawBoundingBox(glm::mat4* viewMatrix)
{
	if (this->model != nullptr)
	{
		Bbox boundingBox = getBoundingBox(viewMatrix);

		glLineWidth(5);
		glBegin(GL_LINES);
		glColor3f(1, 0, 0);

		//Bottom
		glm::vec4 v2 = glm::vec4(boundingBox.mMin.x, boundingBox.mMin.y, boundingBox.mMin.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);
		v2 = glm::vec4(boundingBox.mMax.x, boundingBox.mMin.y, boundingBox.mMin.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);

		v2 = glm::vec4(boundingBox.mMax.x, boundingBox.mMin.y, boundingBox.mMin.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);
		v2 = glm::vec4(boundingBox.mMax.x, boundingBox.mMin.y, boundingBox.mMax.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);

		v2 = glm::vec4(boundingBox.mMin.x, boundingBox.mMin.y, boundingBox.mMax.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);
		v2 = glm::vec4(boundingBox.mMax.x, boundingBox.mMin.y, boundingBox.mMax.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);

		v2 = glm::vec4(boundingBox.mMin.x, boundingBox.mMin.y, boundingBox.mMin.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);
		v2 = glm::vec4(boundingBox.mMin.x, boundingBox.mMin.y, boundingBox.mMax.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);

		//Sides
		v2 = glm::vec4(boundingBox.mMin.x, boundingBox.mMin.y, boundingBox.mMin.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);
		v2 = glm::vec4(boundingBox.mMin.x, boundingBox.mMax.y, boundingBox.mMin.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);

		v2 = glm::vec4(boundingBox.mMax.x, boundingBox.mMin.y, boundingBox.mMin.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);
		v2 = glm::vec4(boundingBox.mMax.x, boundingBox.mMax.y, boundingBox.mMin.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);

		v2 = glm::vec4(boundingBox.mMax.x, boundingBox.mMin.y, boundingBox.mMax.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);
		v2 = glm::vec4(boundingBox.mMax.x, boundingBox.mMax.y, boundingBox.mMax.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);

		v2 = glm::vec4(boundingBox.mMin.x, boundingBox.mMin.y, boundingBox.mMax.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);
		v2 = glm::vec4(boundingBox.mMin.x, boundingBox.mMax.y, boundingBox.mMax.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);

		//Top
		v2 = glm::vec4(boundingBox.mMin.x, boundingBox.mMax.y, boundingBox.mMin.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);
		v2 = glm::vec4(boundingBox.mMax.x, boundingBox.mMax.y, boundingBox.mMin.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);

		v2 = glm::vec4(boundingBox.mMax.x, boundingBox.mMax.y, boundingBox.mMin.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);
		v2 = glm::vec4(boundingBox.mMax.x, boundingBox.mMax.y, boundingBox.mMax.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);

		v2 = glm::vec4(boundingBox.mMin.x, boundingBox.mMax.y, boundingBox.mMax.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);
		v2 = glm::vec4(boundingBox.mMax.x, boundingBox.mMax.y, boundingBox.mMax.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);

		v2 = glm::vec4(boundingBox.mMin.x, boundingBox.mMax.y, boundingBox.mMin.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);
		v2 = glm::vec4(boundingBox.mMin.x, boundingBox.mMax.y, boundingBox.mMax.z, 1.0f);
		glVertex3f(v2.x, v2.y, v2.z);

		glEnd();
	}
}

/*
get a objects bouding box
Author: Bas Rops - 09-06-2014
Last edit: <name> - dd-mm-yyyy
*/
Bbox MapObject::getBoundingBox(glm::mat4* viewMatrix)
{
	//TODO optimize, vertices could be a variable[24] in Bbox. Bbox could probably be a variable in MapObject, and then only recalculate every vertex with modelViewMatrix
	Bbox boundingBox = Bbox();

	if (this->model != nullptr)
	{
		boundingBox = model->getBoundingBox();

		glm::vec3 min = boundingBox.mMin;
		glm::vec3 max = boundingBox.mMax;

		glm::mat4 modelViewMatrix = *viewMatrix;

		std::vector<glm::vec4> vertices;
		//Calculate the axis-aligned bounding box of the object

		//Every vertex that needs to be drawn needs to be multiplied by the modelMatrix, so this gets kinda messy
		//Bottom
		glm::vec4 vertex = modelViewMatrix * glm::vec4(min.x, min.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, min.y, min.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(max.x, min.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, min.y, max.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(min.x, min.y, max.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, min.y, max.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(min.x, min.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(min.x, min.y, max.z, 1.0f);
		vertices.push_back(vertex);

		//Sides
		vertex = modelViewMatrix * glm::vec4(min.x, min.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(min.x, max.y, min.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(max.x, min.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, max.y, min.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(max.x, min.y, max.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, max.y, max.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(min.x, min.y, max.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(min.x, max.y, max.z, 1.0f);
		vertices.push_back(vertex);

		//Top
		vertex = modelViewMatrix * glm::vec4(min.x, max.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, max.y, min.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(max.x, max.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, max.y, max.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(min.x, max.y, max.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, max.y, max.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(min.x, max.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(min.x, max.y, max.z, 1.0f);
		vertices.push_back(vertex);

		glm::vec4 min2 = vertices[0];
		glm::vec4 max2 = vertices[0];

		for each (glm::vec4 vec in vertices)
		{
			min2 = glm::min(min2, vec);
			max2 = glm::max(max2, vec);
		}

		boundingBox.bounds[0] = boundingBox.mMin = glm::vec3(min2);
		boundingBox.bounds[1] = boundingBox.mMax = glm::vec3(max2);
	}

	return boundingBox;
}

Bbox MapObject::getBoundingBoxWithOutViewMatrix()
{
	//TODO optimize, vertices could be a variable[24] in Bbox. Bbox could probably be a variable in MapObject, and then only recalculate every vertex with modelViewMatrix
	Bbox boundingBox = Bbox();

	if (this->model != nullptr)
	{
		boundingBox = model->getBoundingBox();


		glm::vec3 min = boundingBox.mMin;
		glm::vec3 max = boundingBox.mMax;

		std::vector<glm::vec4> vertices;
		//Calculate the axis-aligned bounding box of the object
		glm::mat4 modelViewMatrix =  this->modelMatrix;
		modelViewMatrix = glm::translate(modelViewMatrix, this->position);
		//modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		//modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(180.0f), glm::vec3(0, 1, 0));
		//modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(this->rotation.z), glm::vec3(0, 0, 1));
		//modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(this->rotation.x), glm::vec3(1, 0, 0));
		//modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(-this->rotation.y), glm::vec3(0, 1, 0));

		//Calculate the axis-aligned bounding box of the object

		//Every vertex that needs to be drawn needs to be multiplied by the modelMatrix, so this gets kinda messy
		//Bottom
		glm::vec4 vertex = modelViewMatrix * glm::vec4(min.x, min.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, min.y, min.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(max.x, min.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, min.y, max.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(min.x, min.y, max.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, min.y, max.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(min.x, min.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(min.x, min.y, max.z, 1.0f);
		vertices.push_back(vertex);

		//Sides
		vertex = modelViewMatrix * glm::vec4(min.x, min.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(min.x, max.y, min.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(max.x, min.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, max.y, min.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(max.x, min.y, max.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, max.y, max.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(min.x, min.y, max.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(min.x, max.y, max.z, 1.0f);
		vertices.push_back(vertex);

		//Top
		vertex = modelViewMatrix * glm::vec4(min.x, max.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, max.y, min.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(max.x, max.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, max.y, max.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(min.x, max.y, max.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(max.x, max.y, max.z, 1.0f);
		vertices.push_back(vertex);

		vertex = modelViewMatrix * glm::vec4(min.x, max.y, min.z, 1.0f);
		vertices.push_back(vertex);
		vertex = modelViewMatrix * glm::vec4(min.x, max.y, max.z, 1.0f);
		vertices.push_back(vertex);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//glm::vec4 vertex =  glm::vec4(min.x, min.y, min.z, 1.0f);
		//vertices.push_back(vertex);
		//vertex =  glm::vec4(max.x, min.y, min.z, 1.0f);
		//vertices.push_back(vertex);

		//vertex =  glm::vec4(max.x, min.y, min.z, 1.0f);
		//vertices.push_back(vertex);
		//vertex =  glm::vec4(max.x, min.y, max.z, 1.0f);
		//vertices.push_back(vertex);

		//vertex =  glm::vec4(min.x, min.y, max.z, 1.0f);
		//vertices.push_back(vertex);
		//vertex =  glm::vec4(max.x, min.y, max.z, 1.0f);
		//vertices.push_back(vertex);

		//vertex =  glm::vec4(min.x, min.y, min.z, 1.0f);
		//vertices.push_back(vertex);
		//vertex =  glm::vec4(min.x, min.y, max.z, 1.0f);
		//vertices.push_back(vertex);

		////Sides
		//vertex =  glm::vec4(min.x, min.y, min.z, 1.0f);
		//vertices.push_back(vertex);
		//vertex =  glm::vec4(min.x, max.y, min.z, 1.0f);
		//vertices.push_back(vertex);

		//vertex =  glm::vec4(max.x, min.y, min.z, 1.0f);
		//vertices.push_back(vertex);
		//vertex =  glm::vec4(max.x, max.y, min.z, 1.0f);
		//vertices.push_back(vertex);

		//vertex =  glm::vec4(max.x, min.y, max.z, 1.0f);
		//vertices.push_back(vertex);
		//vertex =  glm::vec4(max.x, max.y, max.z, 1.0f);
		//vertices.push_back(vertex);

		//vertex =  glm::vec4(min.x, min.y, max.z, 1.0f);
		//vertices.push_back(vertex);
		//vertex =  glm::vec4(min.x, max.y, max.z, 1.0f);
		//vertices.push_back(vertex);

		////Top
		//vertex =  glm::vec4(min.x, max.y, min.z, 1.0f);
		//vertices.push_back(vertex);
		//vertex =  glm::vec4(max.x, max.y, min.z, 1.0f);
		//vertices.push_back(vertex);

		//vertex =  glm::vec4(max.x, max.y, min.z, 1.0f);
		//vertices.push_back(vertex);
		//vertex =  glm::vec4(max.x, max.y, max.z, 1.0f);
		//vertices.push_back(vertex);

		//vertex =  glm::vec4(min.x, max.y, max.z, 1.0f);
		//vertices.push_back(vertex);
		//vertex =  glm::vec4(max.x, max.y, max.z, 1.0f);
		//vertices.push_back(vertex);

		//vertex =  glm::vec4(min.x, max.y, min.z, 1.0f);
		//vertices.push_back(vertex);
		//vertex =  glm::vec4(min.x, max.y, max.z, 1.0f);
		//vertices.push_back(vertex);


		glm::vec4 min2 = vertices[0];
		glm::vec4 max2 = vertices[0];

		for each (glm::vec4 vec in vertices)
		{
			min2 = glm::min(min2, vec);
			max2 = glm::max(max2, vec);
		}

		boundingBox.bounds[0] = boundingBox.mMin = glm::vec3(min2);
		boundingBox.bounds[1] = boundingBox.mMax = glm::vec3(max2);
	}

	return boundingBox;
}