#include "IPhysBody.h"
#include "PhysBody.h"
#include "PhysBody_MeshConcave.h"
#include "PhysicsWorld.h"

CPhysicsWorld g_Physics;

/*
============================

	CPhysicsWorld

============================
*/
void CPhysicsWorld::Init()
{
	core.Init();
	SetGravity( -9.81f );
}

void CPhysicsWorld::RegisterPhysBody( IPhysBody& body )
{
	AddRigidBody( body.GetRigidBody() );
	//physicsBodies.push_back( &body );
}

void CPhysicsWorld::RemovePhysBody( IPhysBody& body )
{
	RemoveRigidBody( body.GetRigidBody() );
}
