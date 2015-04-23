#pragma once
#include "AssimpMesh.h"

#include <CaveLib\VBO.h>
#include <CaveLib\Vertex.h>
#include <CaveLib\bbox.h>
#include <CaveLib\Shader.h>
#include <GL\glew.h>
#include <GL\GL.h>
#include <Assimp\Importer.hpp>

#include <string>
#include <vector>

#include "CrimeScene.h"

class ShaderProgram;
struct aiScene;
struct aiNode;

class AssimpModel
{
	Assimp::Importer importer;
	const aiScene* model;
	std::vector<AssimpMesh*> meshes;
	Bbox boundingBox;

	const aiScene* importModel(std::string fileName);
	void loadModel(std::string folderName, const aiScene* model, const aiNode* node, float scale, aiMatrix4x4 parentMatrix, std::map<std::string, cTexture*>* loadedTextures);

public:
	AssimpModel(std::string folderName, std::string fileName, float scale, std::map<std::string, cTexture*>* loadedTextures);
	~AssimpModel();

	Bbox getBoundingBox();
	void draw(Shader<CrimeScene::Uniforms>* shader, glm::mat4* modelMatrix);
};

