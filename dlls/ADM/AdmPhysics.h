/*

======= adm_physics.h ===========================
	________________________
	19th January 2019
	________________________
	AdmSrc Physics Interface
	________________________
	Header file
	________________________

*/

#pragma once

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "ConvexDecomposition/ConvexDecomposition.h"
#include "ConvexDecomposition/cd_wavefront.h"

class CPhysicsWorld
{
public:

						~CPhysicsWorld();

	void				Init(void);

	void				AddRigidBody( btRigidBody* body );
	void				AddCollisionShape( btCollisionShape* colshape );
	void				StepSimulation( float deltaTime, int steps );

	void				SetGravity(float x, float y, float z);
	void				SetGravity(float z);

	bool				HasWorldCollision() { return physWorldShape != nullptr; }
	void				PostUpdate() { }

	void				CreateWorldCollision(const char* path);

	// To-do:
	// Read a BSP
	// Fix jittery box physics
	// Add physics wireframe mode
private:
	btDefaultCollisionConfiguration* physCollisionConfig;

	// default collision dispatcher
	btCollisionDispatcher* physDispatcher;

	// general purpose broadphase, btAxis3Sweep is also an alternative
	btBroadphaseInterface* overlappingPairCache;

	// default constraint solver, this guy's gonna do quite some work
	btSequentialImpulseConstraintSolver* physSolver;

	// the world, the universe, everything that we're in
	btDiscreteDynamicsWorld* world;

	// the array of all collision shapes
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	// serializer, I dunno what we really need it for :P
	btDefaultSerializer* serializer;
	static constexpr int maxSerializeSize = 1024 * 1024 * 5;

	// stuff for map geometry loading
	btTriangleIndexVertexArray* colonVertexArrays;
	btBvhTriangleMeshShape* physWorldShape;
//	btCollisionShape*	groundShape;
	btTransform			groundTrans;

	btDefaultMotionState* groundMS;
	btRigidBody*		groundBody;

	ConvexDecomposition::WavefrontObj objLoader;

	int iCollObjIndex = 0;
};

// Function to convert from units to metres
float utom(float units);

// Function to convert from metres to units
float mtou(float metres);

extern CPhysicsWorld g_Physics; // The base is everywhere
extern string_t g_iszWorldModel; // e.g. mapname.bsp
