#include <functional>
#include "Physics.h"


Physics::Physics()
{
}

Physics::~Physics()
{
	delete world;
	delete solver;
	delete collisionConfiguration;
	delete dispatcher;
	delete broadphase;
	delete playerBody;
	borders.empty();
}

void Physics::AddObjectToWorld(btRigidBody* rigidBody){
	world->addRigidBody(rigidBody);
	rigidBody->activate();
}

void Physics::RemoveObjectFromWorld(btRigidBody* rigidBody)
{
	world->removeRigidBody(rigidBody);
}

void Physics::WorldInit(){
	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver();
	world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	world->setGravity(btVector3(0, -9.8, 0));
	//debugdrawer
	m_pDebugDrawer = new DebugDrawer();
	m_pDebugDrawer->setDebugMode(3);
	world->setDebugDrawer(m_pDebugDrawer);
	FloorInit();
}

void Physics::AddWorldBorders(float xmin, float xmax, float zmin, float zmax){
	float lengteX = xmax - xmin;
	float lengteZ = zmax - zmin;
	btBoxShape * xonder = new btBoxShape(btVector3(lengteX, 100, 0.1));
	btBoxShape * xboven = new btBoxShape(btVector3(lengteX, 100, 0.1));
	btBoxShape * zonder = new btBoxShape(btVector3(0.1, 100, lengteZ));
	btBoxShape * zboven = new btBoxShape(btVector3(0.1, 100, lengteZ));
	btMotionState* x1 = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(xmin, -30, zmin)));
	btMotionState* x2 = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(xmax, -30, zmin)));
	btMotionState* x3 = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(xmax, -30, zmax)));
	btMotionState* x4 = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(xmin, -30, zmax)));

	borders.push_back(new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0,x1, xonder,btVector3(0,0,0))));
	borders.push_back(new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0, x2, xboven, btVector3(0, 0, 0))));
	borders.push_back(new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0, x3, zonder, btVector3(0, 0, 0))));
	borders.push_back(new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0, x4, zboven, btVector3(0, 0, 0))));
	for (btRigidBody * b : borders){
		world->addCollisionObject(b);
		btVector3 b2 = b->getWorldTransform().getOrigin();
		printf("%f %f %f\n", b2.x(), b2.y(), b2.z());
	}
	auto a = world->getCollisionObjectArray();
}

//roationY is in radialen
void Physics::PlayerInit(glm::vec4 position, float rotationY){
	float mass = 3.0f;//kg
	btVector3 fallInertia;
	btBoxShape* pBoxShape = new btBoxShape(btVector3(0.3,0.3,0.3));
	btMotionState* m_pMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x, position.y+1, position.z)));
	pBoxShape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_pMotionState, pBoxShape, fallInertia);
	playerBody = new btRigidBody(rbInfo);
	btQuaternion newRotation = btQuaternion(btVector3(0, 1, 0), rotationY);
	playerBody->getWorldTransform().setRotation(newRotation);
	world->addRigidBody(playerBody);
	playerBody->setFriction(2.0);
}

void Physics::FloorInit(){
	float mass = 0;//kg
	btVector3 fallInertia;
	btBoxShape* pBoxShape = new btBoxShape(btVector3(1000, 0.1, 1000));
	btMotionState* m_pMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(-500, -1.5, -500)));
	pBoxShape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_pMotionState, pBoxShape, fallInertia);
	floor = new btRigidBody(rbInfo);
	world->addCollisionObject(floor);
}

/*
timFctr = gepasseerde tijd in ms
movement = de vector die wordt vermenigvuldigd met de kracht. het resultaat daarvan wordt toegepast op de speler
newRotation = hoek in radialen om heen te draaien
*/
void Physics::UpdateWorld(const float timeFctr, btVector3& movement, const float newRotation){
	float FORCE = 500.0f;//moet worden vervangen door een constante ergens
	btVector3 move = movement*FORCE*timeFctr;
	move = move.rotate(btVector3(0, 1, 0), newRotation);
	playerBody->clearForces();
	playerBody->applyCentralForce(move);
	playerBody->activate();
	//dit moet in een tickcallback staan volgens http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Code_Snippets#I_want_to_cap_the_speed_of_my_spaceship
	btVector3 velocity = playerBody->getLinearVelocity();
	btScalar speed = velocity.length();
	if (speed > FORCE/2) {
		velocity *= FORCE / speed / 2;
		playerBody->setLinearVelocity(velocity);
	}
	//
	world->stepSimulation(timeFctr);
	//printf("move %f,%f,%f angle %f \n", move.x(), move.y(), move.z(),newRotation);
}