#pragma once
#include <CaveLib\Cubemap.h>
#include <CaveLib\Shader.h>
#include <irrKlang.h>

#include <vector>
#include <map>

#include <glm\glm.hpp>

#include "CrimeScene.h"
#include "Physics.h"

class Player;
class MapTiles;
class MapObject;
class cTexture;
class AssimpModel;

class Map
{
	CrimeScene* crimeScene;
	irrklang::ISound* backgroundMusic;

	std::map<std::string, cTexture*> loadedTextures;
	std::map<std::string, AssimpModel*> loadedModels;
	std::vector<MapObject*> mapObjects;
	Cubemap* cubemap;
	MapTiles* mapTiles;
public:
	Map(CrimeScene* crimeScene);
	~Map();

	bool load(std::string mapFileName, ::string filename, Player* player, Physics* physics);
	void loadCubemap(std::string textureFolder);
	void removeMapobject(MapObject* mapobject);
	irrklang::ISound* getBackgroundMusic();
	const std::vector<MapObject*> GetMapObjects();
	void draw(Shader<CrimeScene::Uniforms>* shader,glm::mat4* ViewMatrix);
	void drawCubemap(glm::mat4* projectionMatrix, glm::mat4* viewMatrix);
	void drawBoundingBoxes(glm::mat4* viewMatrix);
	std::string getFileFolder(std::string fileName);
};