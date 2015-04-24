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
}

int Physics::AddObjectToWorld(btRigidBody* rigidBody){
	world->addRigidBody(rigidBody);
	rigidBody->activate();
	bodysInWorld.push_back(rigidBody);
	return bodysInWorld.size() - 1;
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
	
}

//roationY is in radialen
void Physics::PlayerInit(glm::vec4 position, float rotationY){
	float mass = 70.0f;//kg
	btVector3 fallInertia;
	btBoxShape* pBoxShape = new btBoxShape(btVector3(0.3, 1.9, 0.2));
	btMotionState* m_pMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x, position.y, position.z)));
	pBoxShape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_pMotionState, pBoxShape, fallInertia);
	playerBody = new btRigidBody(rbInfo);
	btQuaternion newRotation = btQuaternion(btVector3(0, 1, 0), rotationY);
	playerBody->getWorldTransform().setRotation(newRotation);
	playerBody->setFriction(1);
	world->addRigidBody(playerBody);
}

/*
timFctr = gepasseerde tijd in ms
movement = de vector die wordt vermenigvuldigd met de kracht. het resultaat daarvan wordt toegepast op de speler
newRotation = hoek in radialen om heen te draaien
*/
void Physics::UpdateWorld(const float timeFctr, const glm::vec3 movement,const float newRotation){
	float FORCE = 10.0f;//moet worden vervangen door een constante ergens
	btVector3 move(movement.x, movement.y, movement.z);
	move *= FORCE*timeFctr/1000;
	move = move.rotate(btVector3(0, 1, 0), newRotation);
	playerBody->applyForce(move,btVector3(0,0,-0.9));
	playerBody->activate();
	//dit moet in een tickcallback staan volgens http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Code_Snippets#I_want_to_cap_the_speed_of_my_spaceship
	btVector3 velocity = playerBody->getLinearVelocity();
	btScalar speed = velocity.length();
	if (speed > FORCE) {
		velocity *= FORCE / speed;
		playerBody->setLinearVelocity(velocity);
	}
	//
	world->stepSimulation(timeFctr);
	btVector3 b2 = playerBody->getWorldTransform().getOrigin();
	printf("auto %f,%f,%f \n", b2.x(), b2.y(), b2.z());
}