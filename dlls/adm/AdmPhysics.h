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

class CAdmPhysicsBase
{
public:

	// FRICK, I hope this new/delete stuff will work properly, it's really cancerous sometimes
	btDefaultCollisionConfiguration *admp_CollisionConfiguration; // admp -> AdmPhysics

	// default collision dispatcher
	btCollisionDispatcher *admp_Dispatcher;

	// general purpose broadphase, btAxis3Sweep is also an alternative
	btBroadphaseInterface *admp_overlappingPairCache;

	// default constraint solver, this guy's gonna do quite some work
	btSequentialImpulseConstraintSolver *admp_Solver;

	// the world, the universe, everything that we're in
	btDiscreteDynamicsWorld *admp_World;

	// the array of all collision shapes
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	// stuff for map geometry loading
	btTriangleIndexVertexArray *colonVertexArrays;
	btBvhTriangleMeshShape *PhysMeshShape;
	btCollisionShape *groundShape;
	btTransform groundTrans;

	btDefaultMotionState *groundMS;
	btRigidBody *groundBody;

	void Init(void);
	void SetGravity(float x, float y, float z);
	void SetGravity(float z);

	btCollisionObject *NewCollisionObject()
	{
		iCollObjIndex++;
		int i = iCollObjIndex - 1;
		return admp_World->getCollisionObjectArray()[i];
	}

	void CreateShapeWithVerts()
	{

	}

	void CreateWorldCollision(const char* path);

	// To-do:
	// Interface with GoldSrc world coordinates - DONE
	// Read a BSP
private:
	int iCollObjIndex = 0;
};

// Function to convert from units to metres
float utom(float units);

// Function to convert from metres to units
float mtou(float metres);

extern CAdmPhysicsBase AdmPhysEngine; // The base is everywhere
extern string_t g_iszWorldModel; // e.g. mapname.bsp
