#include "Map.h"
#include "CrimeScene.h"
#include "MapTiles.h"
#include "MapObject.h"
#include "AssimpModel.h"
#include "Utils.h"

#include <CaveLib\texture.h>
#include <Cavelib\model.h>
#include <CaveLib\Shader.h>
#include <CaveLib\TextureAtlas.h>
#include <json\reader.h>

#include <iostream>
#include <fstream>
#include <exception>
#include <time.h>

#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#define TEXTURE_FOLDER "data/CrimeSceneV4/Textures/"
#define MODEL_FOLDER "data/CrimeSceneV4/Models/"
#define THEME_FOLDER "data/CrimeSceneV4/Themes/"
#define PLAYER_HEIGHT 1.7f

#pragma region cubemap vertices
const std::vector<glm::vec3> cubemapVertices = {
	glm::vec3(-100.0f, 100.0f, -100.0f),
	glm::vec3(-100.0f, -100.0f, -100.0f),
	glm::vec3(100.0f, -100.0f, -100.0f),
	glm::vec3(100.0f, -100.0f, -100.0f),
	glm::vec3(100.0f, 100.0f, -100.0f),
	glm::vec3(-100.0f, 100.0f, -100.0f),

	glm::vec3(-100.0f, -100.0f, 100.0f),
	glm::vec3(-100.0f, -100.0f, -100.0f),
	glm::vec3(-100.0f, 100.0f, -100.0f),
	glm::vec3(-100.0f, 100.0f, -100.0f),
	glm::vec3(-100.0f, 100.0f, 100.0f),
	glm::vec3(-100.0f, -100.0f, 100.0f),

	glm::vec3(100.0f, -100.0f, -100.0f),
	glm::vec3(100.0f, -100.0f, 100.0f),
	glm::vec3(100.0f, 100.0f, 100.0f),
	glm::vec3(100.0f, 100.0f, 100.0f),
	glm::vec3(100.0f, 100.0f, -100.0f),
	glm::vec3(100.0f, -100.0f, -100.0f),

	glm::vec3(-100.0f, -100.0f, 100.0f),
	glm::vec3(-100.0f, 100.0f, 100.0f),
	glm::vec3(100.0f, 100.0f, 100.0f),
	glm::vec3(100.0f, 100.0f, 100.0f),
	glm::vec3(100.0f, -100.0f, 100.0f),
	glm::vec3(-100.0f, -100.0f, 100.0f),

	glm::vec3(-100.0f, 100.0f, -100.0f),
	glm::vec3(100.0f, 100.0f, -100.0f),
	glm::vec3(100.0f, 100.0f, 100.0f),
	glm::vec3(100.0f, 100.0f, 100.0f),
	glm::vec3(-100.0f, 100.0f, 100.0f),
	glm::vec3(-100.0f, 100.0f, -100.0f),

	glm::vec3(-100.0f, -100.0f, -100.0f),
	glm::vec3(-100.0f, -100.0f, 100.0f),
	glm::vec3(100.0f, -100.0f, -100.0f),
	glm::vec3(100.0f, -100.0f, -100.0f),
	glm::vec3(-100.0f, -100.0f, 100.0f),
	glm::vec3(100.0f, -100.0f, 100.0f)
};
#pragma endregion

/*
Constructor
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 03-06-2014
*/
Map::Map(CrimeScene* crimeScene)
{
	this->crimeScene = crimeScene;

	this->loadedTextures = std::map<std::string, cTexture*>();
	this->loadedModels = std::map<std::string, AssimpModel*>();
	this->mapObjects = std::vector<MapObject*>();
}

/*
Destructor
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 22-05-2014
*/
Map::~Map()
{
	delete this->cubemap;
	delete this->mapTiles;

	for each (auto loadedTexture in loadedTextures)
	{
		delete loadedTexture.second;
	}
	this->loadedTextures.clear();

	for each (auto loadedModel in loadedModels)
	{
		delete loadedModel.second;
	}
	this->loadedModels.clear();

	for each (MapObject* mapObject in mapObjects)
	{
		delete mapObject;
	}
	this->mapObjects.clear();

	if (backgroundMusic)
		backgroundMusic->drop();
}

/*
Load a map
file = the file of the map to load
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 10-06-2014
*/
bool Map::load(std::string file, Player* player)
{
	Json::Value root;
	Json::Reader reader;

	//Read the file
	std::string data = "";
	std::string line;
	std::ifstream pFile(file);
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
		std::cout << "Failed to open file " << file << ". Please make sure the config command is in the correct format: '--map <something.json>'" << std::endl;;
		return false;
	}

	//Parse the data as JSON
	bool parsingSuccessful = reader.parse(data, root);
	if (!parsingSuccessful)
	{
		std::cout << "Failed to parse json " << file << "\n" << reader.getFormattedErrorMessages();
		return false;
	}

#pragma region load tiles
	//Set the root JSON tiles
	const Json::Value tiles = root["tiles"];

	//Loop through each object in objects and create a MapTile for each
	for (unsigned int index = 0; index < tiles.size(); ++index)
	{
		std::string textureFileName = tiles[index].get("texture", "").asString();
		float x = tiles[index].get("x", 0.0f).asFloat();
		float y = tiles[index].get("y", 0.0f).asFloat();
		float z = tiles[index].get("z", 0.0f).asFloat();

		if (this->mapTiles == nullptr)
			this->mapTiles = new MapTiles();

		mapTiles->addTile(TEXTURE_FOLDER + std::string("tiles/") + textureFileName, glm::vec3(x, y - PLAYER_HEIGHT, -z), glm::vec3(1.0f, 0.0f, 1.0f));
	}

	if (this->mapTiles != nullptr)
		mapTiles->setVBO();
#pragma endregion

#pragma region load objects
	//Set the root JSON objects
	const Json::Value objects = root["objects"];

	//Loop through each object in objects and create a MapObject for each
	for (unsigned int index = 0; index < objects.size(); ++index)
	{
		std::string modelFileName = objects[index].get("model", "").asString();
		std::string category = objects[index].get("category", "").asString();
		float x = objects[index].get("x", 0.0f).asFloat();
		float y = objects[index].get("y", 0.0f).asFloat();
		float z = objects[index].get("z", 0.0f).asFloat();
		float rotationX = objects[index].get("rotationX", 0.0f).asFloat();
		float rotationY = objects[index].get("rotationY", 0.0f).asFloat();
		float rotationZ = objects[index].get("rotationZ", 0.0f).asFloat();
		float scale = objects[index].get("scale", 1.0f).asFloat();
		bool interactable = objects[index].get("interactable", false).asBool();
		bool standardVisible = objects[index].get("standardVisible", true).asBool();
		std::string description = objects[index].get("description", "").asString();

		float mass;
		if (category == "Walls" || category == "Road" || category == "Traces"  || category == "Roofs")
		{
			mass = 0.0f;
		}
		else if (category == "Walls")
		{
			mass = -1.0f;
		}
		else
		{
			mass = 1.0f;
		}

		if (loadedModels.count(modelFileName) <= 0)
		{
			std::string modelLocation = MODEL_FOLDER + (category != "" ? category + '/' : "") + getFileFolder(modelFileName);
			loadedModels[modelFileName] = new AssimpModel(modelLocation, modelFileName, scale, &loadedTextures);
		}

		AssimpModel* model = loadedModels[modelFileName];

		mapObjects.push_back(new MapObject(model, glm::vec3(x, y - PLAYER_HEIGHT, -z), glm::vec3(rotationX, rotationY, rotationZ),
											scale, interactable, standardVisible, description,mass));
	}


#pragma endregion

#pragma region load scene info
	//Set the root JSON object
	const Json::Value sceneInfo = root["sceneInfo"];

	std::string sceneTheme = sceneInfo.get("sceneTheme", "").asString();
	float playerPositionX = sceneInfo.get("playerPositionX", 0.0f).asFloat();
	float playerPositionY = sceneInfo.get("playerPositionY", 0.0f).asFloat();
	float playerPositionZ = sceneInfo.get("playerPositionZ", 0.0f).asFloat();
	float playerRotationY = sceneInfo.get("playerRotationY", 0.0f).asFloat();

	if (sceneTheme != "")
	{
		loadCubemap(THEME_FOLDER + sceneTheme);
		backgroundMusic = crimeScene->playSound2D(THEME_FOLDER + sceneTheme + '/' + sceneTheme + ".mp3", true, true);
	}

	//Set player position and rotation
	player->setPosition(glm::vec4(-playerPositionX, playerPositionY, -playerPositionZ, 1.0f));
	player->setRotation(playerRotationY);
#pragma endregion

	pFile.close();

	//Map successfully loaded
	return true;
}

/*
removes the given MapOjbect 
Author:Tycho Krielaart - 23-05-2014
Last edit: <name> - dd-mm-yyyy
*/
void Map::removeMapobject(MapObject* mapobject)
{
	mapObjects.erase(std::remove(mapObjects.begin(), mapObjects.end(), mapobject), mapObjects.end());
}

/*
Load a cubemap
textureFolder = location of the folder that contains the 6 cubemap textures (posx.jpg, negx.jpg, posy.jpg, negy.jpg, posz.jps, negz.jpg)
Author: Bas Rops - 22-05-2014
Last edit: <name> - dd-mm-yyyy
*/
void Map::loadCubemap(std::string textureFolder)
{
	std::string filenames[] = { textureFolder + "/posx.jpg", textureFolder + "/negx.jpg", textureFolder + "/posy.jpg",
								textureFolder + "/negy.jpg", textureFolder + "/posz.jpg", textureFolder + "/negz.jpg" };

	//Load the cubemap shader
	ShaderProgram* shader = new ShaderProgram("data/CrimeSceneV4/Shaders/cubemap.vert", "data/CrimeSceneV4/Shaders/cubemap.frag");
	shader->bindAttributeLocation("a_position", 0);
	shader->link();

	//Create a new cubemap and generate a VBO and VAO
	cubemap = new Cubemap(shader, filenames, &cubemapVertices);
}

/*
Retrieve a pointer to the background music of this Map
Author: Bas Rops - 10-06-2014
Last edit: <name> - dd-mm-yyyy
*/
irrklang::ISound* Map::getBackgroundMusic()
{
	return this->backgroundMusic;
}

/*
Retrieve a vector containing pointers to all MapObjects
Author: Tycho krielaart - ??-??-2014
Last edit: Bas Rops - 10-06-2014
*/
const std::vector<MapObject*> Map::GetMapObjects()
{
	return mapObjects;
}

/*
Draw all objects of the map
shader = shader to draw the model with
Author: Bas Rops - 25-04-2014
Last edit: Bas Rops - 05-06-2014
*/
void Map::draw(Shader<CrimeScene::Uniforms>* shader)
{
	if (mapTiles != nullptr)
	{
		//Set shader uniforms for tiles, so that they're not set in every mapTile->draw
		shader->setUniform(CrimeScene::Uniforms::useTexture, 1);
		//shader->setUniform(CrimeScene::Uniforms::scale, 1); //TODO?
		shader->setUniform(CrimeScene::Uniforms::objectVisible, true);
		shader->setUniform(CrimeScene::Uniforms::meshModelMatrix, glm::mat4());

		mapTiles->draw(shader);
	}

	//Some meshes do not have textures unless back face culling is enabled
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//Use counter clockwise for objects
	glFrontFace(GL_CCW);

	for each (MapObject* mapObject in mapObjects)
	{
		mapObject->draw(shader);
	}

	glDisable(GL_CULL_FACE);
}

/*
Draw the map's cubemap
Author: Bas Rops - 22-05-2014
Last edit: <name> - dd-mm-yyyy
*/
void Map::drawCubemap(glm::mat4* projectionMatrix, glm::mat4* viewMatrix)
{
	if (cubemap)
		cubemap->draw(projectionMatrix, viewMatrix);
}

/*
Draws the boundingboxes of all objects
Author: Bas Rops - 20-05-2014
Last edit: <name> - dd-mm-yyyy
*/
void Map::drawBoundingBoxes(glm::mat4* viewMatrix)
{
	for each (MapObject* mapObject in mapObjects)
	{
		mapObject->drawBoundingBox(viewMatrix);
	}
}

/*
Retrieve the name of the folder the file is located in
fileName = name of the file with it's extension
Author: Bas Rops - 08-05-2014
Last edit: Bas Rops - 09-05-2014
*/
std::string Map::getFileFolder(std::string fileName)
{
	std::string folderName;
	std::vector<std::string> folderStrings = Utils::splitString(fileName, ".");
	if (folderStrings.size() > 2)
	{
		for (unsigned int i = 0; i < folderStrings.size() - 1; i++)
		{
			folderName += folderStrings[i];
			if (i < folderStrings.size() - 2)
				folderName += '.';
		}
	}
	else
		folderName = folderStrings[0];

	return std::string(folderName + '/');
}