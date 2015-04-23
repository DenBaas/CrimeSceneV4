#pragma once
#include <CaveLib\VBO.h>
#include <CaveLib\VAO.h>
#include <CaveLib\Vertex.h>
#include <CaveLib\bbox.h>
#include <CaveLib\Shader.h>
#include <assimp\scene.h>

#include <map>
#include <string>

#include "CrimeScene.h"

class cMaterial;
class cTexture;

class AssimpMesh
{
	VBO<VertexPositionNormalTexture>* vbo;
	VAO<VertexPositionNormalTexture>* vao;

	glm::mat4 meshMatrix;
	float scale;
	cMaterial* material;
	Bbox boundingBox;

	int nonTriangles;

public:
	AssimpMesh(std::string folderName, const aiScene* model, const aiMesh* mesh, aiMatrix4x4 meshMatrix, float scale, std::map<std::string, cTexture*>* loadedTextures);
	~AssimpMesh();

	void loadMesh(const aiMesh* mesh);
	void loadMaterial(std::string folderName, const aiMaterial *mtl, std::map<std::string, cTexture*>* loadedTextures);
	Bbox getBoundingBox();
	int getNonTriangles();

	glm::mat4 getMeshMatrix();
	void draw(Shader<CrimeScene::Uniforms>* shader, glm::mat4* modelMatrix);
};

