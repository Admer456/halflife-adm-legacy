#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"

#include "../shared/ADM/Physics/IPhysBody.h"
#include "../shared/ADM/Physics/PhysBody.h"
#include "../shared/ADM/Physics/PhysBody_Box.h"

#include "PhysBox.h"

LINK_ENTITY_TO_CLASS( phys_box, CPhysBox );

void CPhysBox::Spawn()
{
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	if ( pev->model )
	{
		SET_MODEL( ENT( pev ), STRING( pev->model ) );
	}

	physBody.Init( this, params );

	pev->nextthink = gpGlobals->time + 1.f / 60.f;
}

void CPhysBox::KeyValue( KeyValueData* pkvd )
{
	if ( KeyvalueToken( physMass ) )
	{
		KeyvalueToFloat( params.mass );
	}
	else if ( KeyvalueToken( physExtents ) )
	{
		KeyvalueToVector( params.dimensions );
	}
	else if ( KeyvalueToken( physOriginOffset ) )
	{
		KeyvalueToVector( params.offsetOrigin )
	}
	else if ( KeyvalueToken( physAngleOffset ) )
	{
		KeyvalueToVector( params.offsetAngles )
	}
	else
	{
		KeyvaluesFromBase( CBaseEntity );
	}
}

void CPhysBox::Think()
{
	// TO-DO:
	// Instead of directly setting the origin,
	// we could calculate and set pev->velocity
	// and that way, we'll avoid clipping into players
	// Same story for angles, I suppose?
	pev->origin = physBody.GetOrigin();
	pev->angles = physBody.GetAngles();

	pev->nextthink = gpGlobals->time + 1.f / 60.f;
}

void CPhysBox::Touch( CBaseEntity* other )
{
	// Apply some force if the player is pushing
	if ( !other->IsPlayer() )
		return;

	Vector delta = pev->origin - other->pev->origin;
	Vector force = delta.Normalize() * (1.0f / delta.Length());

	btVector3 btDelta( delta.x, delta.z, delta.y );
	btVector3 btForce( force.x, force.z, force.y );
	
	btDelta = -PhysUtils::utom(btDelta);
	btForce = PhysUtils::utom(btForce);

	physBody.GetRigidBody()->applyImpulse( btForce, btDelta );
}
