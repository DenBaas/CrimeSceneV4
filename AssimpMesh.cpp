#include "AssimpMesh.h"
#include "Utils.h"

#include <CaveLib\Shader.h>
#include <CaveLib\material.h>
#include <CaveLib\texture.h>
#include <assimp\scene.h>

#include <iostream>
#include <glm\gtc\matrix_transform.hpp>

/*
Constructor
Loads the material of a mesh and loops through all faces of a mesh, adding them to a VBO. Only triangles are accepted for now.
folderName = location of the folder to load textures from
model = pointer to the model that is being loaded
mesh = pointer to the mesh that is being loaded
loadedTextures = pointer to a map of strings and cTextures that contains the already loaded textures
Author: Bas Rops - 09-05-2014
Last edit: Bas Rops - 06-06-2014
*/
AssimpMesh::AssimpMesh(std::string folderName, const aiScene* model, const aiMesh* mesh, aiMatrix4x4 meshMatrix, float scale, std::map<std::string, cTexture*>* loadedTextures)
{
	loadMaterial(folderName, model->mMaterials[mesh->mMaterialIndex], loadedTextures);

	//Convert aiMatrix4x4 to glm::mat4, transpose it and set it as the mesh's meshMatrix
	this->meshMatrix = glm::transpose(glm::mat4(meshMatrix.a1, meshMatrix.a2, meshMatrix.a3, meshMatrix.a4,
		meshMatrix.b1, meshMatrix.b2, meshMatrix.b3, meshMatrix.b4,
		meshMatrix.c1, meshMatrix.c2, meshMatrix.c3, meshMatrix.c4,
		meshMatrix.d1, meshMatrix.d2, meshMatrix.d3, meshMatrix.d4));

	this->scale = scale;
	//Scale the meshMatrix by the mesh's scale
	this->meshMatrix = glm::scale(glm::mat4(), glm::vec3(this->scale, this->scale, this->scale)) * this->meshMatrix;

	//Load the aiMesh into AssimpMesh
	loadMesh(mesh);
}

/*
Destructor
Author: Bas Rops - 09-05-2014
Last edit: <name> - dd-mm-yyyy
*/
AssimpMesh::~AssimpMesh()
{
	delete this->vbo;
	delete this->vao;
}

/*
Convert data from aiMesh to AssimpMesh
Author: Bas Rops - 06-06-2014
Last edit: <name> - dd-mm-yyyy
*/
void AssimpMesh::loadMesh(const aiMesh* mesh)
{
	glm::vec3 min;
	glm::vec3 max;

	std::vector<VertexPositionNormalTexture> vertices;

	//Loop through all faces in the mesh
	for (unsigned int t = 0; t < mesh->mNumFaces; t++)
	{
		const struct aiFace* face = &mesh->mFaces[t];
		GLenum face_mode;

		switch (face->mNumIndices)
		{
		case 1: face_mode = GL_POINTS;
			break;
		case 2: face_mode = GL_LINES;
			break;
		case 3: face_mode = GL_TRIANGLES;
			break;
		default: face_mode = GL_POLYGON;
			break;
		}

		//Ignore points, lines and polygons
		if (face_mode != GL_TRIANGLES)
		{
			nonTriangles++;
			continue;
		}

		glm::vec3 faceVertex;
		glm::vec3 faceNormal;
		glm::vec2 faceTexcoord;

		//Go through all vertices in face
		for (unsigned int i = 0; i < face->mNumIndices; i++)
		{
			//Get group index for current index
			int vertexIndex = face->mIndices[i];

			//Check what the min and max coordinates are for this mesh
			glm::vec3 vertex = glm::vec3(this->meshMatrix * glm::vec4(mesh->mVertices[vertexIndex].x, mesh->mVertices[vertexIndex].y, mesh->mVertices[vertexIndex].z, 1));
			if (t - nonTriangles > 0 || i > 0)
			{
				min = glm::min(vertex, min);
				max = glm::max(vertex, max);
			}
			else
				min = max = vertex;

			//If the model already has normals, use those. Else calculate them.
			if (mesh->mNormals != NULL)
				faceNormal = glm::vec3(mesh->mNormals[vertexIndex].x, mesh->mNormals[vertexIndex].y, mesh->mNormals[vertexIndex].z);
			else
			{
				//Calculate the normals
				glm::vec3 vertex0;
				glm::vec3 vertex1;
				glm::vec3 vertex2;

				switch (i)
				{
				case 0:
					vertex0 = glm::vec3(mesh->mVertices[vertexIndex].x, mesh->mVertices[vertexIndex].y, mesh->mVertices[vertexIndex].z);
					vertex1 = glm::vec3(mesh->mVertices[face->mIndices[i + 1]].x, mesh->mVertices[face->mIndices[i + 1]].y, mesh->mVertices[face->mIndices[i + 1]].z);
					vertex2 = glm::vec3(mesh->mVertices[face->mIndices[i + 2]].x, mesh->mVertices[face->mIndices[i + 2]].y, mesh->mVertices[face->mIndices[i + 2]].z);
					break;
				case 1:
					vertex0 = glm::vec3(mesh->mVertices[face->mIndices[i - 1]].x, mesh->mVertices[face->mIndices[i - 1]].y, mesh->mVertices[face->mIndices[i - 1]].z);
					vertex1 = glm::vec3(mesh->mVertices[vertexIndex].x, mesh->mVertices[vertexIndex].y, mesh->mVertices[vertexIndex].z);
					vertex2 = glm::vec3(mesh->mVertices[face->mIndices[i + 1]].x, mesh->mVertices[face->mIndices[i + 1]].y, mesh->mVertices[face->mIndices[i + 1]].z);
					break;
				case 2:
					vertex0 = glm::vec3(mesh->mVertices[face->mIndices[i - 2]].x, mesh->mVertices[face->mIndices[i - 2]].y, mesh->mVertices[face->mIndices[i - 2]].z);
					vertex1 = glm::vec3(mesh->mVertices[face->mIndices[i - 1]].x, mesh->mVertices[face->mIndices[i - 1]].y, mesh->mVertices[face->mIndices[i - 1]].z);
					vertex2 = glm::vec3(mesh->mVertices[vertexIndex].x, mesh->mVertices[vertexIndex].y, mesh->mVertices[vertexIndex].z);
					break;
				default:
					break;
				}

				faceNormal = glm::normalize(glm::cross((vertex1 - vertex0), (vertex2 - vertex0)));
			}

			//Get the texture coordinates, if there are any
			if (mesh->HasTextureCoords(0))
				faceTexcoord = glm::vec2(mesh->mTextureCoords[0][vertexIndex].x, 1 - mesh->mTextureCoords[0][vertexIndex].y);
			else
				faceTexcoord = glm::vec2(0.0f, 0.0f);

			//Get the vertices of the face
			faceVertex = glm::vec3(mesh->mVertices[vertexIndex].x, mesh->mVertices[vertexIndex].y, mesh->mVertices[vertexIndex].z);

			//Push a new VertexPositionNormalTexture in vertices
			vertices.push_back(VertexPositionNormalTexture(faceVertex, faceNormal, faceTexcoord));

			//If the model has no normals of itself, the normals were calculated. Smooth those normals below
			//Disabled, because it creates some... weird artifacts because *all* normals of the same position are used for calculating.
			//Sometimes, you don't want smooth normals...
			//Good luck with that (You could do something like smooth normals if the angle between the normals is less than 40 degrees
			//if (mesh->mNormals == NULL)
			//{
			//	std::vector<glm::vec3> tmpPositions;
			//	std::vector<glm::vec3> tmpNormals;

			//	//Retrieve the normals of the vertices and add the normals of the vertices that are the same
			//	for (unsigned int i = 0; i < vertices.size(); i++)
			//	{
			//		VertexPositionNormalTexture& tmpVertex = vertices[i];
			//		glm::vec3& tmpPos = tmpVertex.position;

			//		//Check if the position was added to tmpVertices already
			//		std::vector<glm::vec3>::iterator it = std::find_if(tmpPositions.begin(), tmpPositions.end(),
			//			[tmpPos](const glm::vec3 n){return n.x == tmpPos.x && n.y == tmpPos.y && n.z == tmpPos.z; });
			//		if (it != tmpPositions.end())
			//		{
			//			//Does contain, increment the normal
			//			size_t index = it - tmpPositions.begin();
			//			tmpNormals[index] += tmpVertex.normal;
			//		}
			//		else
			//		{
			//			//Does not contain, push new values
			//			tmpPositions.push_back(tmpVertex.position);
			//			tmpNormals.push_back(tmpVertex.normal);
			//		}
			//	}

			//	//Normalize all normals
			//	for (unsigned int i = 0; i < tmpNormals.size(); i++)
			//	{
			//		//If tmpNormal is 0.0f, 0.0f, 0.0f, the vector can not be normalized (duh)
			//		if (tmpNormals[i].x > 0.0f || tmpNormals[i].y > 0.0f || tmpNormals[i].z > 0)
			//			tmpNormals[i] = glm::normalize(tmpNormals[i]);
			//	}

			//	//Place the smoothed normals back into the VertexPositionNormalTexture
			//	for (unsigned int i = 0; i < vertices.size(); i++)
			//	{
			//		glm::vec3 tmpPos = vertices[i].position;
			//		std::vector<glm::vec3>::iterator it = std::find_if(tmpPositions.begin(), tmpPositions.end(),
			//			[tmpPos](const glm::vec3 n){return n.x == tmpPos.x && n.y == tmpPos.y && n.z == tmpPos.z; });
			//		if (it != tmpPositions.end())
			//		{
			//			size_t index = it - tmpPositions.begin();
			//			vertices[i].normal = tmpNormals[index];
			//		}
			//	}
			//}
		}
	}

	//Set the boundingbox for this mesh
	this->boundingBox = Bbox(min, max);

	//Create our VBO
	if (vertices.size() > 0)
	{
		vbo = new VBO<VertexPositionNormalTexture>();

		vbo->bind();
		vbo->setData(vertices.size(), &vertices[0], GL_STATIC_DRAW);
		vertices.clear();
	}
}

/*
Load a cMaterial from the mesh's aiMaterial, so that it is compatible with drawing in the CAVE
folderName = name of the folder to load textures from
mtl = pointer to the aiMaterial of the model
loadedTextures = pointer to a map of strings and cTextures that contains the already loaded textures
Author: Bas Rops - 09-05-2014
Last edit: Bas Rops - 27-05-2014
*/
void AssimpMesh::loadMaterial(std::string folderName, const aiMaterial *mtl, std::map<std::string, cTexture*>* loadedTextures)
{
	material = NULL;

	aiColor4D diffuse;
	aiColor4D specular;
	aiColor4D ambient;
	aiColor4D transparency;

	int texIndex = 0;
	aiString texPath;

	if (mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath) == aiReturn_SUCCESS)
	{
		if (material == NULL)
			material = new cMaterial();

		std::vector<std::string> subStrings = Utils::splitString(folderName, "/");
		std::string textureFolderName = subStrings[subStrings.size() - 1];
		std::string textureFolderFileName = folderName + '/' + texPath.C_Str();

		if (loadedTextures->count(textureFolderFileName) <= 0)
			(*loadedTextures)[textureFolderFileName] = CaveLib::loadTexture(folderName + texPath.C_Str());

		material->tDiffuse = (*loadedTextures)[textureFolderFileName];
	}

	if (aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == aiReturn_SUCCESS)
	{
		if (material == NULL)
			material = new cMaterial();

		material->diffuse = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
	}

	if (aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular) == aiReturn_SUCCESS)
	{
		if (material == NULL)
			material = new cMaterial();

		material->specular = glm::vec3(specular.r, specular.g, specular.b);
	}

	if (aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient) == aiReturn_SUCCESS)
	{
		if (material == NULL)
			material = new cMaterial();

		material->ambient = glm::vec3(ambient.r, ambient.g, ambient.b);
	}

	if (aiGetMaterialColor(mtl, AI_MATKEY_COLOR_TRANSPARENT, &transparency) == aiReturn_SUCCESS)
	{
		if (material == NULL)
			material = new cMaterial();

		material->transparency = transparency.a;
	}
}

/*
Retrieve the boundingbox of this mesh
Author: Bas Rops - 20-05-2014
Last edit: <name> - dd-mm-yyyy
*/
Bbox AssimpMesh::getBoundingBox()
{
	return this->boundingBox;
}

/*
Debug method for retrieving the amount of faces within this mesh that are not triangles
Author: Bas Rops - 09-05-2014
Last edit: <name> - dd-mm-yyyy
*/
int AssimpMesh::getNonTriangles()
{
	return nonTriangles;
}

/*
Retrieve the meshMatrix of this mesh
Author: Bas Rops - 13-05-2014
Last edit: <name> - dd-mm-yyyy
*/
glm::mat4 AssimpMesh::getMeshMatrix()
{
	return this->meshMatrix;
}

/*
Draw the mesh
Author: Bas Rops - 09-05-2014
Last edit: Bas Rops - 22-05-2014
*/
void AssimpMesh::draw(Shader<CrimeScene::Uniforms>* shader, glm::mat4* modelMatrix)
{
	if (material)
	{
	//TODO	material->apply(shader);
		if (material->tDiffuse)
			glBindTexture(GL_TEXTURE_2D, material->tDiffuse->tid());
		if (material->tAmbient)
			glBindTexture(GL_TEXTURE_2D, material->tAmbient->tid());
		if (material->tSpecular)
			glBindTexture(GL_TEXTURE_2D, material->tSpecular->tid());

		shader->setUniform(CrimeScene::Uniforms::useTexture, material->tDiffuse != NULL || material->tAmbient != NULL || material->tSpecular != NULL);

		shader->setUniform(CrimeScene::Uniforms::matAmbient, material->ambient);
		shader->setUniform(CrimeScene::Uniforms::matDiffuse, material->diffuse);
		shader->setUniform(CrimeScene::Uniforms::matSpecular, material->specular);
		shader->setUniform(CrimeScene::Uniforms::matShinyness, material->shinyness);

		shader->setUniform(CrimeScene::Uniforms::matTransparency, material->transparency);

	}

	if (vbo)
	{
		if (shader)
		{
			shader->setUniform(CrimeScene::Uniforms::meshModelMatrix, *modelMatrix * this->meshMatrix);

			if (vao == NULL)
				vao = new VAO<VertexPositionNormalTexture>(vbo);

			vao->bind();
			glDrawArrays(GL_TRIANGLES, 0, vbo->getLength());
			vao->unBind();
		}
		else
		{
			vbo->bind();
			vbo->setPointer();
			glDrawArrays(GL_TRIANGLES, 0, vbo->getLength());
			vbo->unsetPointer();
			vbo->unBind();
		}
	}
}
