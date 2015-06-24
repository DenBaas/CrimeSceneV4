#pragma once
#ifdef _DEBUG
#pragma comment(lib, "Bullet3Common_vs2010_debug.lib")
#pragma comment(lib, "Bullet3Collision_vs2010_debug.lib")
#pragma comment(lib, "Bullet3Dynamics_vs2010_debug.lib")
#pragma comment(lib, "BulletCollision_vs2010_debug.lib")
#pragma comment(lib, "BulletDynamics_vs2010_debug.lib")
#pragma comment(lib, "Bullet3Geometry_vs2010_debug.lib")
#pragma comment(lib, "LinearMath_vs2010_debug.lib")
#else
#pragma comment(lib, "Bullet3Common_vs2010.lib")
#pragma comment(lib, "Bullet3Collision_vs2010.lib")
#pragma comment(lib, "Bullet3Dynamics_vs2010.lib")
#pragma comment(lib, "BulletCollision_vs2010.lib")
#pragma comment(lib, "BulletDynamics_vs2010.lib")
#pragma comment(lib, "Bullet3Geometry_vs2010.lib")
#pragma comment(lib, "LinearMath_vs2010.lib")
#endif
#include <btBulletDynamicsCommon.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <CaveLib\model.h>
#include <irrKlang.h>

#include "DebugDrawer.h"

using namespace std;

class Physics
{
public:
	Physics();
	~Physics();

	btBroadphaseInterface*                  broadphase;
	btDefaultCollisionConfiguration*        collisionConfiguration;
	btCollisionDispatcher*                  dispatcher;
	btSequentialImpulseConstraintSolver*    solver;
	btDiscreteDynamicsWorld*                world;
	btBvhTriangleMeshShape*					levelShape;
	btCollisionObject *						level;
	btRigidBody *							playerBody;
	btRigidBody *							floor;
	std::vector<btRigidBody *> borders;

	void PlayerInit(glm::vec4 position, float rotationY);
	void WorldInit();
	void FloorInit();
	void RemoveObjectFromWorld(btRigidBody* rigidBody);
	void UpdateWorld(const float timeFctr, btVector3& movement,const float newRotation);
	void AddObjectToWorld(btRigidBody* rigidBody);
	void AddWorldBorders(float xmin, float xmax, float zmin, float zmax);

private:
	DebugDrawer * m_pDebugDrawer;
};

