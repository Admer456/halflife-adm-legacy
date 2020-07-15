#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"

#include "IPhysBody.h"
#include "PhysBody.h"
#include "PhysicsWorld.h"

using namespace PhysUtils;

void CPhysBody::Init( CBaseEntity* parent, const PhysParameters& params )
{
	this->parent = parent;
	mass = params.mass;

	isDynamic = mass != 0.0f;

	ResetTransform();
	SetOrigin( parent->pev->origin );
	SetAngles( parent->pev->angles );

	motionState = new btDefaultMotionState( transform );

	InitShape( params );

	rigidBody = ConstructRigidBody( mass, motionState, shape );

	if ( rigidBody )
	{
		g_Physics.RegisterPhysBody( *this );
	}
}

void CPhysBody::ResetTransform()
{
	transform.setIdentity();
}

void CPhysBody::SetOrigin( const btVector3& origin )
{
	transform.setOrigin( origin );
}

void CPhysBody::SetAngles( const btVector3& angles )
{
	transform.setRotation( btQuaternion(angles.x(), angles.y(), angles.z()) );
}

void CPhysBody::SetOrigin( const Vector& origin )
{
	btVector3 btOrigin( origin.x, origin.z, origin.y );
	transform.setOrigin( utom(btOrigin) );
}

void CPhysBody::SetAngles( const Vector& angles )
{
	btQuaternion btAngles( angles.y, angles.x, angles.z );
	transform.setRotation( btAngles * (M_PI/180.f) );
}

Vector CPhysBody::GetOrigin()
{
	btVector3 btOrigin = transform.getOrigin();

	return Vector( btOrigin.x(), btOrigin.z(), btOrigin.y() );
}

Vector CPhysBody::GetAngles()
{
	Vector angles;

	transform.getRotation().getEulerZYX( angles.y, angles.x, angles.z );

	return angles * (180.f/M_PI);
}
