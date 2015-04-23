#pragma once
#include <Cavelib\Vertex.h>
#include <Cavelib\VAO.h>
#include <Cavelib\VBO.h>
#include <CaveLib\Shader.h>

#include <glm\glm.hpp>

#include "CrimeScene.h"

class cTexture;
class TextureAtlas;

class MapTiles
{
	TextureAtlas* tileAtlas;
	std::vector<VertexPositionNormalTexture> vertices;
	VBO<VertexPositionNormalTexture>* vboTile;

public:
	MapTiles();
	~MapTiles();

	void addTile(std::string fileName, glm::vec3 position, glm::vec3 dimensions);
	void setVBO();

	void draw(Shader<CrimeScene::Uniforms>* shader);
};

