#include <glm\glm.hpp>

#include <CaveLib/Shader.h>
#include "CrimeScene.h"

#define CHANGE_ZOOM 0.01f
#define ROTATION_SPEED 0.2f

class MapObject;
class PositionalDevice;
class Player;

class InspectObject
{
	glm::vec4 position;
	glm::mat4 originalModelMatrix;
	glm::mat4 angles;
	glm::vec3 zoom;
	MapObject* object;
	glm::mat4 inspectMatrix;

public:
	InspectObject(MapObject* object, glm::vec4 position);
	~InspectObject();

	void draw(Shader<CrimeScene::Uniforms>* shader,glm::mat4* ViewMatrix);
	void updateView(glm::mat4 changeMatrix, bool zoomIn, bool zoomOut, Player* player, PositionalDevice* head);
	MapObject* getInspectedObject();
};

