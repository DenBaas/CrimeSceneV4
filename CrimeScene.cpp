#include "CrimeScene.h"
#include "ToolboxPanel.h"
#include "Map.h"
#include "InspectObject.h"
#include "MapObject.h"
#include "Photo.h"
#include "Utils.h"
#include "RestApi.h"

#include <Cavelib\texture.h>
#include <Cavelib\model.h>
#include <CaveLib\ray.h>
#include <CaveLib\Shader.h>
#include <CaveLib\GUIPanel.h>
#include <VrLib\Kernel.h>
#include <CaveLib\bbox.h>

#include <iostream>
#include <vector>
#include <conio.h>	//Console I/O, required for _getch()

#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\matrix_inverse.hpp>
#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <time.h>

#define _USE_MATH_DEFINES
#include <math.h>

#define CONE_LENGTH 4.0f
#define CONE_NEAR_RADIUS 0.025f
#define CONE_FAR_RADIUS 0.5f

//crimescene, oculus viewport 25fps
//crimescene, simulator 45fps
// after shader optimalisation
//crimescene, oculus viewport 33fps
//crimescene, simulator 69fps


/*
Entry point for the application
Use '--map <something.json>' to load a map
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 13-06-2014
*/
int main(int argc, char* argv[])
{
	Kernel* kernel = Kernel::getInstance();
	RestApi::getInstance();
	CrimeScene* application = nullptr;

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "--config") == 0)
		{
			i++;
			kernel->loadConfig(argv[i]);
		}
		if (strcmp(argv[i], "--map") == 0)
		{
			i++;
			application = new CrimeScene(argv[i]);
		}
	}


	if (application == nullptr)
		application = new CrimeScene("");

	//RestApi::getInstance()->registerAsEnvironment();

	kernel->setApp(application);
	kernel->start();

	delete application;

	return 0;
}

/*
Constructor
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 05-06-2014
*/
CrimeScene::CrimeScene(std::string filename)
{
	this->mapFilename = filename;
}

/*
Destructor
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 13-06-2014
*/
CrimeScene::~CrimeScene()
{
	if (retrievedObjects.size() > 0)
	{
		std::cout << "Verzamelde objecten/sporen:" << std::endl;
		for each (MapObject* object in retrievedObjects)
		{
			std::cout << "- " << object->getDescription() << '\n' << std::endl;
		}
		/*InventoryFileIO saveItems;
		saveItems.save(DataFormat::DEBUG, &retrievedObjects);*/
		SetForegroundWindow(GetConsoleWindow());
		std::cout << "Press any key to exit..." << std::endl;
		_getch();
	}

	delete map;
	delete toolboxPanel;
	delete player;
	delete inspectingObject;

	delete shaderDefault;
	delete shaderPolylight;
	delete shaderSpotlightCone;
	delete physics;

	soundEngine->drop();
}

/*
Exit the application after receiving input in the console window
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
void CrimeScene::consoleExit()
{
	SetForegroundWindow(GetConsoleWindow());
	std::cout << "Press any key to exit..." << std::endl;
	_getch();
	exit(0);
}

/*
Initializer
Author: Bas Rops - 25-04-2014
Last edit: Ricardo Blommers - 03-06-2014
*/
void CrimeScene::init()
{
	initDevices();
	initOpenGL();
	initSpotlight();
	initShaders();

	//Initialize irrKlang sound engine
	soundEngine = irrklang::createIrrKlangDevice();
	if (!soundEngine)
	{
		std::cout << "Error starting SoundEngine." << std::endl;
		consoleExit();
	}

	player = new Player();

	toolboxPanel = new ToolboxPanel(this);

	map = new Map(this);
	if (!map->load("data/CrimeSceneV4/CrimeScenes/" + this->mapFilename, player))
		consoleExit();
	irrklang::ISound* music = map->getBackgroundMusic();
	if (music != nullptr && music->getIsPaused())
		music->setIsPaused(false);

	photo = new Photo(1024, 1024);
	physics = new Physics();
	
	//physics->WorldInit(map->GetMapObjects());
	//physics->PlayerInit(player->getPosition(), player->getRotation());
}

/*
Play a 2D sound. Sound engine needs to be initialized.
filename = location of the sound to play
loop = loop the sound or not
startPaused = start the sound paused
track = if startPaused == false, set this to true to still retrieve a pointer to the played sound
StartPaused or track needs to be true to get a pointer to the ISound
Author: Bas Rops - 23-05-2014
Last edit: <name> - dd-mm-yyyy
*/
irrklang::ISound* CrimeScene::playSound2D(std::string filename, bool loop, bool startPaused, bool track)
{
	if (soundEngine)
		return soundEngine->play2D(filename.c_str(), loop, startPaused, track);
	else
	{
		std::cout << "Sound engine == NULL, can not play sound." << std::endl;
		return NULL;
	}
}

/*
"Update-function" that does stuff before the draw function gets called
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 11-06-2014
*/
void CrimeScene::preFrame(double frameTime, double totalTime)
{
	clock_t clock_end = clock();
	GLfloat timeFctr = GLfloat(clock_end - clock_start) / CLOCKS_PER_SEC; // calculate time(s) elapsed since last frame
	clock_start = clock();
	toolboxPanel->setSelector(wandRay);

	handleInput(frameTime);

	//Only update the toolbox when an object is being inspected
	if (inspectingObject)
		updateInspectingObject();
	//physics->UpdateWorld(timeFctr,glm::vec3(1,1,1),0);
}

/*
Handles input from wand, buttons, keyboard presses, etc
Author: Bas Rops - 11-06-2014
Last edit: <name> - dd-mm-yyyy
*/
void CrimeScene::handleInput(float elapsedTime)
{
#pragma region Wand
	if (hydraLeft.isInitialized())
	{
		wandMatrix = hydraRightNunchuck.getData();
		wandPosition = wandMatrix * glm::vec4(0, 0, 0, 1);
		glm::vec4 point = wandMatrix * glm::vec4(0, 0, -1, 1);
		glm::vec4 wandDiff = point - wandPosition;

		wandRay = Ray(glm::vec3(wandPosition[0], wandPosition[1], wandPosition[2]), glm::vec3(wandDiff[0], wandDiff[1], wandDiff[2]));
		wandTarget = wandRay.mOrigin + (wandRay.mDir * 100.0f);
	}
	else
	{
		wandMatrix = wand.getData();
		wandPosition = wandMatrix * glm::vec4(0, 0, 0, 1);
		glm::vec4 point = wandMatrix * glm::vec4(0, 0, -1, 1);
		glm::vec4 wandDiff = point - wandPosition;

		wandRay = Ray(glm::vec3(wandPosition[0], wandPosition[1], wandPosition[2]), glm::vec3(wandDiff[0], wandDiff[1], wandDiff[2]));
		wandTarget = wandRay.mOrigin + (wandRay.mDir * 100.0f);
	}
#pragma endregion

#pragma region Left button
	if (leftButton.getData() == TOGGLE_ON)
	{
		//If an object is being inspected, add it to the list of retrieved objects and remove it from the scene
		if (inspectingObject)
		{
			MapObject* object = inspectingObject->getInspectedObject();
			delete inspectingObject;
			map->removeMapobject(object);
			retrievedObjects.push_back(object);

			inspectingObject = nullptr;
		}
		//Else toggle the polylight
		else
			isUsingPolylight = !isUsingPolylight;
	}
#pragma endregion

#pragma region Right button
	if (RightButton.getData() == TOGGLE_ON)
	{
		//If an object is being inspected, put it back and stop inspecting
		if (inspectingObject)
		{
			delete inspectingObject;
			inspectingObject = nullptr;
		}
		//Check which interactable object the ray has collision with. If there is one, set that object as inspectingItem
		else
		{
			std::vector<MapObject*> objects = map->GetMapObjects();

			for each (MapObject* object in objects)
			{
				//TODO check closest item instead of first
				Bbox box = object->getBoundingBox(&player->getPlayerMatrix());
				if (object->getIsInteractable() && box.hasRayCollision(wandRay, 0.0f, 10.0f))
				{
					inspectingObject = new InspectObject(object, player->getPosition());
					toolboxPanel->setDescription(object->getDescription());
					break;
				}
			}
		}
	}
#pragma endregion

#pragma region Page down button
	if (PageDownButton.getData() == TOGGLE_ON && !inspectingObject)
		createScreenshot();
#pragma endregion

#pragma region player movement
	if (!inspectingObject)
		if (hydraLeft.isInitialized())
		{
			player->update(hydraLeft.getData(), hydraRight.getData(), elapsedTime);
		}
		else
			player->update(head.getData());
#pragma endregion

}

/*
Update the view of inspectingObject and the position of the toolboxPanel
Author: Tycho Krielaart - ??-??-2014
Last edit: Bas Rops - 11-06-2014
*/
void CrimeScene::updateInspectingObject()
{
	glm::mat4 mat = wand.getData();

	mat[3][0] = 1;
	mat[3][1] = 1;
	mat[3][2] = 1;
	mat[3][3] = 1;

	bool zoomIn = PageUpButton.getData() == DigitalState::ON;
	bool zoomOut = PageDownButton.getData() == DigitalState::ON;

	inspectingObject->updateView(mat, zoomIn, zoomOut, player, &head);

	toolboxPanel->updatePosition(&head);
}


/*
Draw the scene/all objects using a custom view matrix
projectionMatrix = the projection matrix to use for drawing
viewMatrix = the view matrix to use for drawing
Author: Bas Rops - 25-04-2014
Last edit: Ricardo Blommers - 09-06-2014
*/
void CrimeScene::draw(const glm::mat4 &projectionMatrix, const glm::mat4 &modelViewMatrix)
{
	glm::mat4 viewMatrix = modelViewMatrix * player->getPlayerMatrix();

	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(0);

	//TODO fix color
	drawWand();
	//physics.world->debugDrawWorld();
	//Only draw the axis and boundingboxes in debug mode
#ifdef _DEBUG
	drawAxis();
	map->drawBoundingBoxes(&viewMatrix);
#endif // _DEBUG

	//Draw the map's cubemap
	map->drawCubemap(const_cast<glm::mat4*>(&projectionMatrix), &viewMatrix);

	if (isUsingPolylight)
		drawMapWithPolylight(const_cast<glm::mat4*>(&projectionMatrix), &viewMatrix);
	else
		drawMap(const_cast<glm::mat4*>(&projectionMatrix), &viewMatrix);

	//Draw the toolboxpanel when inspecting an item
	if (inspectingObject)
	{
		glEnable(GL_CULL_FACE);
		//Use clockwise for the toolbox
		glFrontFace(GL_CW);
		toolboxPanel->draw();
		glDisable(GL_CULL_FACE);
	}
}

/*
Draw all objects in the map
projectionMatrix = the projection matrix to use for drawing
viewMatrix = the view matrix to use for drawing
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 22-05-2014
*/
void CrimeScene::drawMap(glm::mat4* projectionMatrix, glm::mat4* viewMatrix)
{
	shaderDefault->use();
	shaderDefault->setUniform(Uniforms::viewProjectionMatrix, *projectionMatrix * *viewMatrix);
	//Don't forget to set the modelMatrix when drawing the object!

	if (map != nullptr)
		map->draw(shaderDefault);

	if (inspectingObject != nullptr)
		inspectingObject->draw(shaderDefault);
}

/*
Draw all objects in the map using a polylight shader
projectionMatrix = the projection matrix to use for drawing
viewMatrix = the view matrix to use for drawing
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 22-05-2014
*/
void CrimeScene::drawMapWithPolylight(glm::mat4* projectionMatrix, glm::mat4* viewMatrix)
{
	shaderPolylight->use();
	shaderPolylight->setUniform(Uniforms::projectionMatrix, *projectionMatrix);
	shaderPolylight->setUniform(Uniforms::viewMatrix, *viewMatrix);
	shaderPolylight->setUniform(Uniforms::wandPosition, wandRay.mOrigin);
	shaderPolylight->setUniform(Uniforms::wandDirection, wandRay.mDir);
	shaderPolylight->setUniform(Uniforms::spotCutoff, atan2((CONE_FAR_RADIUS), CONE_LENGTH) * (180.0f / (float)M_PI));
	shaderPolylight->setUniform(Uniforms::maxLength, CONE_LENGTH);
	//Don't forget to set the modelMatrix when drawing the object!
	
	if (map != nullptr)
		map->draw(shaderPolylight);

	//TODO draw with default shader? Might be better.
	if (inspectingObject != nullptr)
		inspectingObject->draw(shaderPolylight);
	
	shaderSpotlightCone->use();
	shaderSpotlightCone->setUniform(SpotlightConeUniforms::wandModelProjectionMatrix, *projectionMatrix * wandMatrix);
	vboPolylight->bind();
	vboPolylight->setPointer();
	glDrawArrays(GL_TRIANGLES, 0, vboPolylight->getLength());
	vboPolylight->unsetPointer();
	vboPolylight->unBind();
}

/*
Initializes CAVE devices
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
void CrimeScene::initDevices()
{
	wand.init("WandPosition");
	head.init("MainUserHead");
	leftButton.init("LeftButton");
	RightButton.init("RightButton");
	PageDownButton.init("KeyPageDown");
	PageUpButton.init("KeyPageUp");

	hydraRightNunchuck.init("RightNunchuk");
	hydraLeft.init("LeftJoystick");
	hydraRight.init("RightJoystick");

}

/*
Initializes OpenGL functions
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 20-05-2014
*/
void CrimeScene::initOpenGL()
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_COLOR_MATERIAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	
	this->clearColor = glm::vec4(0.3f, 0.8f, 0.7f, 1.0f);
}

/*
Loads and initializes shaders
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
void CrimeScene::initShaders()
{
	shaderDefault = new Shader<Uniforms>("data/CrimeSceneV4/Shaders/default.vert", "data/CrimeSceneV4/Shaders/default.frag");
	shaderDefault->bindAttributeLocation("a_position", 0);
	shaderDefault->bindAttributeLocation("a_normal", 1);
	shaderDefault->bindAttributeLocation("a_texcoord", 2);
	shaderDefault->link();

	shaderDefault->registerUniform(Uniforms::viewProjectionMatrix, "viewProjectionMatrix");
	shaderDefault->registerUniform(Uniforms::meshModelMatrix, "meshModelMatrix");
	shaderDefault->registerUniform(Uniforms::objectVisible, "objectVisible");
	shaderDefault->registerUniform(Uniforms::useTexture, "useTexture");
	shaderDefault->registerUniform(Uniforms::matAmbient, "matAmbient");
	shaderDefault->registerUniform(Uniforms::matDiffuse, "matDiffuse");
	shaderDefault->registerUniform(Uniforms::matSpecular, "matSpecular");
	shaderDefault->registerUniform(Uniforms::matShinyness, "matShinyness");
	shaderDefault->registerUniform(Uniforms::matTransparency, "matTransparency");

	shaderDefault->registerUniform(Uniforms::projectionMatrix, "projectionMatrix");
	shaderDefault->registerUniform(Uniforms::viewMatrix, "viewMatrix");
	shaderDefault->registerUniform(Uniforms::meshModelMatrix, "meshModelMatrix");
	shaderDefault->registerUniform(Uniforms::wandPosition, "wandPosition");
	shaderDefault->registerUniform(Uniforms::spotCutoff, "spotCutoff");
	shaderDefault->registerUniform(Uniforms::maxLength, "maxLength");
	shaderDefault->registerUniform(Uniforms::wandDirection, "wandDirection");
	shaderDefault->registerUniform(Uniforms::s_texture, "s_texture");

	shaderSpotlightCone = new Shader<SpotlightConeUniforms>("data/CrimeSceneV4/Shaders/spotlightCone.vert", "data/CrimeSceneV4/Shaders/spotlightCone.frag");
	shaderSpotlightCone->bindAttributeLocation("a_position", 0);
	shaderSpotlightCone->bindAttributeLocation("a_normal", 1);
	shaderSpotlightCone->link();

	shaderSpotlightCone->registerUniform(SpotlightConeUniforms::wandModelProjectionMatrix, "wandModelProjectionMatrix");
	shaderSpotlightCone->registerUniform(SpotlightConeUniforms::wandPosition, "wandPosition");

	shaderPolylight = new Shader<Uniforms>("data/CrimeSceneV4/Shaders/objectPolylight.vert", "data/CrimeSceneV4/Shaders/objectPolylight.frag");
	shaderPolylight->bindAttributeLocation("a_position", 0);
	shaderPolylight->bindAttributeLocation("a_normal", 1);
	shaderPolylight->bindAttributeLocation("a_texcoord", 2);
	shaderPolylight->link();


	shaderPolylight->registerUniform(Uniforms::viewProjectionMatrix, "viewProjectionMatrix");
	shaderPolylight->registerUniform(Uniforms::meshModelMatrix, "meshModelMatrix");
	shaderPolylight->registerUniform(Uniforms::objectVisible, "objectVisible");
	shaderPolylight->registerUniform(Uniforms::useTexture, "useTexture");
	shaderPolylight->registerUniform(Uniforms::matAmbient, "matAmbient");
	shaderPolylight->registerUniform(Uniforms::matDiffuse, "matDiffuse");
	shaderPolylight->registerUniform(Uniforms::matSpecular, "matSpecular");
	shaderPolylight->registerUniform(Uniforms::matShinyness, "matShinyness");
	shaderPolylight->registerUniform(Uniforms::matTransparency, "matTransparency");

	shaderPolylight->registerUniform(Uniforms::projectionMatrix, "projectionMatrix");
	shaderPolylight->registerUniform(Uniforms::viewMatrix, "viewMatrix");
	shaderPolylight->registerUniform(Uniforms::meshModelMatrix, "meshModelMatrix");
	shaderPolylight->registerUniform(Uniforms::wandPosition, "wandPosition");
	shaderPolylight->registerUniform(Uniforms::spotCutoff, "spotCutoff");
	shaderPolylight->registerUniform(Uniforms::maxLength, "maxLength");
	shaderPolylight->registerUniform(Uniforms::wandDirection, "wandDirection");
	shaderPolylight->registerUniform(Uniforms::s_texture, "s_texture");

}

/*
Initializes the spotlight object by calculating and writing vertices to a VBO
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
void CrimeScene::initSpotlight()
{
	std::vector<VertexPositionNormalTexture> vertices;

	float inc = (float)(M_PI / 10);
	float tmult = (float)(4 / M_PI);

	for (float r = 0; r < 2 * M_PI; r += inc)
	{
		//Triangle 1 point 1 (close by)
		vertices.push_back(VertexPositionNormalTexture(glm::vec3(CONE_NEAR_RADIUS*cos(r), CONE_NEAR_RADIUS*sin(r), 0), glm::vec3(cos(r), sin(r), 0), glm::vec2(r*tmult, CONE_LENGTH / 0.05f)));
		//Triangle 1 point 2 (close by)
		vertices.push_back(VertexPositionNormalTexture(glm::vec3(CONE_NEAR_RADIUS*cos(r + inc), CONE_NEAR_RADIUS*sin(r + inc), 0), glm::vec3(cos(r + inc), sin(r + inc), 0), glm::vec2((r + inc)*tmult, CONE_LENGTH / 0.05f)));
		//Triangle 1 point 3 (far away)
		vertices.push_back(VertexPositionNormalTexture(glm::vec3(CONE_FAR_RADIUS*cos(r + inc), CONE_FAR_RADIUS*sin(r + inc), -CONE_LENGTH), glm::vec3(cos(r + inc), sin(r + inc), 0), glm::vec2((r + inc)*tmult, (CONE_LENGTH + 1) / 0.05f)));

		//Triangle 2 point 1 (far away)
		vertices.push_back(VertexPositionNormalTexture(glm::vec3(CONE_FAR_RADIUS*cos(r + inc), CONE_FAR_RADIUS*sin(r + inc), -CONE_LENGTH), glm::vec3(cos(r + inc), sin(r + inc), 0), glm::vec2((r + inc)*tmult, (CONE_LENGTH + 1) / 0.05f)));
		//Triangle 2 point 2 (far away)
		vertices.push_back(VertexPositionNormalTexture(glm::vec3(CONE_FAR_RADIUS*cos(r), CONE_FAR_RADIUS*sin(r), -CONE_LENGTH), glm::vec3(cos(r), sin(r), 0), glm::vec2(r*tmult, (CONE_LENGTH + 1) / 0.05f)));
		//Triangle 2 point 3 (close by)
		vertices.push_back(VertexPositionNormalTexture(glm::vec3(CONE_NEAR_RADIUS*cos(r), CONE_NEAR_RADIUS*sin(r), 0), glm::vec3(cos(r), sin(r), 0), glm::vec2(r*tmult, CONE_LENGTH / 0.05f)));
	}

	//Near circle
	for (float r = 0; r < 2 * M_PI; r += inc)
	{
		vertices.push_back(VertexPositionNormalTexture(glm::vec3(CONE_NEAR_RADIUS*cos(r), CONE_NEAR_RADIUS*sin(r), 0), glm::vec3(0, 0, 1), glm::vec2(0, 0)));
		vertices.push_back(VertexPositionNormalTexture(glm::vec3(CONE_NEAR_RADIUS*cos(r + inc), CONE_NEAR_RADIUS*sin(r + inc), 0), glm::vec3(0, 0, 1), glm::vec2(0, 0)));
		vertices.push_back(VertexPositionNormalTexture(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0)));
	}

	//Far circle
	for (float r = 0; r < 2 * M_PI; r += inc)
	{
		vertices.push_back(VertexPositionNormalTexture(glm::vec3(CONE_FAR_RADIUS*cos(r), CONE_FAR_RADIUS*sin(r), -CONE_LENGTH), glm::vec3(0, 0, 1), glm::vec2(0, 0)));
		vertices.push_back(VertexPositionNormalTexture(glm::vec3(CONE_FAR_RADIUS*cos(r + inc), CONE_FAR_RADIUS*sin(r + inc), -CONE_LENGTH), glm::vec3(0, 0, 1), glm::vec2(0, 0)));
		vertices.push_back(VertexPositionNormalTexture(glm::vec3(0, 0, -CONE_LENGTH), glm::vec3(0, 0, 1), glm::vec2(0, 0)));
	}

	vboPolylight = new VBO<VertexPositionNormalTexture>();

	vboPolylight->bind();
	vboPolylight->setData(vertices.size(), &vertices[0], GL_DYNAMIC_DRAW);
	vertices.clear();
}

/*
Draws the wand ray using old and ugly OpenGL, without shaders
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 20-05-2014
*/
void CrimeScene::drawWand()
{
	glLineWidth(10);
	glBegin(GL_LINES);
	glColor4f(1, 1, 1, 1);
	glVertex3f(wandPosition[0], wandPosition[1], wandPosition[2]);
	glVertex3f(wandTarget[0], wandTarget[1], wandTarget[2]);
	glEnd();
}

/*
Draws the x/y/z-axis using old and ugly OpenGL, without shaders
Author: Bas Rops - 13-05-2014
Last edit: Bas Rops - 20-05-2014
*/
void CrimeScene::drawAxis()
{
	glLineWidth(10);
	glBegin(GL_LINES);
	glColor4f(1, 0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(1, 0, 0);

	glColor4f(0, 1, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1, 0);

	glColor4f(0, 0, 1, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1);
	glEnd();
}

/*
Author: Ricardo Blommers - 03-06-2014
Last edit: Ricardo Blommers - 26-06-2014
*/
void CrimeScene::createScreenshot()
{
	glm::mat4 playerMatrix = player->getPlayerMatrix();
	glm::mat4 viewMatrix = wand.getData();
	glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 1000.0f);

	// Altercations to calculate the viewMatrix
	glm::vec4 playerPosition = playerMatrix * glm::vec4(0, 0, 0, 1);
	glm::vec4 playerDirection = playerMatrix * glm::vec4(0, 0, -1, 1);

	glm::vec4 wandPosition = viewMatrix * glm::vec4(0, 0, 0, 1);
	glm::vec4 wandDirection = viewMatrix * glm::vec4(0, 0, -1, 1);
	glm::vec4 wandUp = viewMatrix * glm::vec4(0, 1, 0, 0);

	glm::vec4 position = wandPosition + playerPosition;
	glm::vec4 direction = wandDirection + playerDirection;

	// With the Cave's matrices generally being a double reverse quadruple backflipped 
	// inverted enigma of a creation, this calculation is employed to flip the 
	// direction of the camera into the right direction
	// (since both vec3's are inverted when creating the matrix)
	glm::vec4 compensation = direction - position;
	glm::vec4 newDirection = position - compensation;
	glm::mat4 view = glm::lookAt(-glm::vec3(position), -glm::vec3(newDirection), glm::vec3(wandUp));

	// Temporarely store the original viewport
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Temporarely replace the viewport to enable larger screenshots
	glViewport(0, 0, 1024, 1024);
	photo->bind();
	draw(proj, view);
	photo->unbind();

	// Reinstate original viewport
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	photo->generateImage();
}