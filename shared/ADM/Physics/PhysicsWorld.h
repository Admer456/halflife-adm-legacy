#pragma once

#include <vector>
#include "BulletPhysics.h"

/*
========================================================

	PhysCore

	Holds basic Bullet initialisation stuff

========================================================
*/
struct PhysCore
{
	btDefaultCollisionConfiguration* physCollisionConfig;

	// default collision dispatcher
	btCollisionDispatcher*		physDispatcher;

	// general purpose broadphase, btAxis3Sweep is also an alternative
	btBroadphaseInterface*		overlappingPairCache;

	// default constraint solver, this guy's gonna do quite some work
	btSequentialImpulseConstraintSolver* physSolver;

	// the world, the universe, everything that we're in
	btDiscreteDynamicsWorld*	world;

	void Init()
	{
		physCollisionConfig = new btDefaultCollisionConfiguration();
		physDispatcher = new btCollisionDispatcher( physCollisionConfig );
		overlappingPairCache = new btDbvtBroadphase();
		physSolver = new btSequentialImpulseConstraintSolver();
		world = new btDiscreteDynamicsWorld( physDispatcher, overlappingPairCache, physSolver, physCollisionConfig );
	}
};

//class BulletDebugDrawer;
class IPhysBody;
//class CPhysBody_ConcaveMesh;

/*
========================================================

	CPhysicsWorld

	Runs the simulation, manages physics bodies etc.

========================================================
*/
class CPhysicsWorld
{
public:
	CPhysicsWorld() = default;
	~CPhysicsWorld() = default;

	void						Init( void );

	void						RegisterPhysBody( IPhysBody& body );
	void						RemovePhysBody( IPhysBody& body );

	// Utility functions
public:
	void						AddRigidBody( btRigidBody* body ) { core.world->addRigidBody( body ); }
	void						RemoveRigidBody( btRigidBody* body ) { core.world->removeRigidBody( body ); }
	
	bool						IsLoaded() { return core.world != nullptr; }
	void						StepSimulation( float deltaTime, int steps ) { core.world->stepSimulation( deltaTime, steps ); }

	// Just a quick reminder, Bullet uses a right-handed 3D coordinate system
	void						SetGravity( float x, float y, float z ) { core.world->setGravity( btVector3( x, z, y ) ); }
	void						SetGravity( float z ) { SetGravity( 0.f, 0.f, z ); }

	// Debug rendering - you'd typically implement a debug renderer in client.dll
	void						DebugDrawWorld() { core.world->debugDrawWorld(); }
	void						SetDebugDrawer( btIDebugDraw* drawer ) { core.world->setDebugDrawer( drawer ); }

private:
	PhysCore					core;			// Stuff required for initialisation
	//std::vector<IPhysBody*>		physicsBodies;	// List of all physics bodies to be simulated
};

extern CPhysicsWorld g_Physics; // The base is everywhere
