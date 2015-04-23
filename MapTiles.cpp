#include "MapTiles.h"

#include <Cavelib\texture.h>
#include <CaveLib\Shader.h>
#include <CaveLib\TextureAtlas.h>

#include <glm\gtc\type_ptr.hpp>

/*
Constructor
Author: Bas Rops - 06-05-2014
Last edit: Bas Rops - 03-06-2014
*/
MapTiles::MapTiles()
{
	this->tileAtlas = new TextureAtlas();
}

/*
Destructor
Author: Bas Rops - 06-05-2014
Last edit: Bas Rops - 03-06-2014
*/
MapTiles::~MapTiles()
{
	delete this->tileAtlas;
	delete this->vboTile;
}

/*
Add a tile
fileName = location of the texture
position = position of the tile
dimensions = dimensions of the tile, dimensions.y is not used and is always 0.0f
Author: Bas Rops - 06-05-2014
Last edit: Bas Rops - 03-06-2014
*/
void MapTiles::addTile(std::string fileName, glm::vec3 position, glm::vec3 dimensions)
{
	TASubTexture* tileTexture = tileAtlas->addTexture(fileName);

	if (tileTexture)
	{
		//TODO fix bleeding if it's there by substracting half a pixel from endCoordinates, not needed for now
		int size = tileAtlas->getTextureSize();
		glm::vec2 startCoordinate = glm::vec2(tileTexture->getStartPosition().x / size, tileTexture->getStartPosition().y / size);
		glm::vec2 endCoordinate = glm::vec2(startCoordinate.x + (tileTexture->getDimensions().x / size), startCoordinate.y + (tileTexture->getDimensions().y / size));

		//Top left
		vertices.push_back(VertexPositionNormalTexture(position + glm::vec3(0.0f, 0.0f, -dimensions.z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(startCoordinate.x, endCoordinate.y)));
		//Top right
		vertices.push_back(VertexPositionNormalTexture(position + glm::vec3(dimensions.x, 0.0f, -dimensions.z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(endCoordinate.x, endCoordinate.y)));
		//Bottom right
		vertices.push_back(VertexPositionNormalTexture(position + glm::vec3(dimensions.x, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(endCoordinate.x, startCoordinate.y)));
		//Bottom left
		vertices.push_back(VertexPositionNormalTexture(position, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(startCoordinate.x, startCoordinate.y)));
	}
}

/*
Binds the vertices of the added tiles to the vbo
Make sure you only call this after all tiles have been added
Author: Bas Rops - 06-05-2014
Last edit: Bas Rops - 03-06-2014
*/
void MapTiles::setVBO()
{
	if (vertices.size() > 0)
	{
		vboTile = new VBO<VertexPositionNormalTexture>();

		vboTile->bind();
		vboTile->setData(vertices.size(), &vertices[0], GL_DYNAMIC_DRAW);
		vertices.clear();
	}
}

/*
Draw the tiles
shader = shader to draw the tile with
Remember to set tile-specific uniforms in this method!
Author: Bas Rops - 06-05-2014
Last edit: Bas Rops - 05-06-2014
*/
void MapTiles::draw(Shader<CrimeScene::Uniforms>* shader)
{
	tileAtlas->bind();
	vboTile->bind();
	vboTile->setPointer();
	vboTile->setVAO();
	glDrawArrays(GL_QUADS, 0, vboTile->getLength());
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	vboTile->unsetPointer();
	vboTile->unBind();
}
