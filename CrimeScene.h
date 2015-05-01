#pragma once
#include "Player.h"
#include "Physics.h"
//#include "InventoryFileIO.h"
#include <VrLib\Application.h>
#include <VrLib\Device.h>
#include <CaveLib\Plane.h>
#include <CaveLib\ray.h>
#include <Cavelib\Vertex.h>
#include <Cavelib\VAO.h>
#include <Cavelib\VBO.h>
#include <CaveLib\Shader.h>

#include <string>
#include <glm\glm.hpp>
#include <GL\glew.h>
#include <irrKlang.h>
#include <ctime>

#include "WiiYourself\WiiMoteWrapper.h"

class cTexture;
class cModel;
class ShaderProgram;
class GUIPanel;
class ToolboxPanel;
class Map;
class MapObject;
class Cubemap;
class InspectObject;
class Photo;

class CrimeScene: public Application
{
	PositionalDevice wand;
	PositionalDevice head;
	DigitalDevice leftButton;
	DigitalDevice RightButton;
	DigitalDevice PageDownButton;
	DigitalDevice PageUpButton;
	PositionalDevice hydraRightNunchuck;

	TwoDimensionDevice hydraLeft;
	TwoDimensionDevice hydraRight;

	std::string mapFilename;
	Map* map;

	Ray wandRay;
	glm::mat4 wandMatrix;
	glm::vec4 wandPosition;
	glm::vec3 wandTarget;

	Physics* physics;
	WiiMoteWrapper * wiimoteData;
	clock_t clock_start;
	int wiimode = 0;//zie indeling knoppen op dropbox wat waar voor staat
	int MAXMODES = 3;
	float rotation = 0;
	float zoomfactor = 0;
public:
	enum class Uniforms
	{
		viewProjectionMatrix,
		projectionMatrix,
		viewMatrix,
		wandPosition,
		wandDirection,
		spotCutoff,
		maxLength,
		meshModelMatrix,
		objectVisible,
		matAmbient,
		useTexture,
		matDiffuse,
		matSpecular,
		matShinyness,
		matTransparency,
		s_texture
	};
private:
	Shader<Uniforms>* shaderDefault;
	Shader<Uniforms>* shaderPolylight;

	enum class SpotlightConeUniforms
	{
		wandModelProjectionMatrix,
		wandPosition
	};
	Shader<SpotlightConeUniforms>* shaderSpotlightCone;

	VBO<VertexPositionNormalTexture>* vboPolylight;

	irrklang::ISoundEngine* soundEngine;
	ToolboxPanel* toolboxPanel;
	Player* player;
	Cubemap* cubemap;
	InspectObject* inspectingObject;
	bool isUsingPolylight;
	bool isInspectingObject;
	std::vector<MapObject*> retrievedObjects;
	Photo* photo;
	
	void createScreenshot();
	void handleInput(float elapsedTime);
	void updateInspectingObject();
	void handleWiiMote();

	void draw(const glm::mat4 &projectionMatrix, const glm::mat4 &modelViewMatrix);
	void drawMap(glm::mat4* projectionMatrix, glm::mat4* viewMatrix);
	void drawMapWithPolylight(glm::mat4* projectionMatrix, glm::mat4* viewMatrix);
	void drawWand();
	void drawAxis();

	void initDevices();
	void initOpenGL();
	void initShaders();
	void initSpotlight();

public:
	CrimeScene(std::string filename, WiiMoteWrapper* w);
	~CrimeScene();

	void consoleExit();
	irrklang::ISound* playSound2D(std::string filename, bool loop, bool startPaused, bool track = true);

	virtual void init();
	virtual void preFrame(double frameTime, double totalTime);
};