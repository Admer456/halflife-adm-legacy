#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"

#include "../shared/ADM/Physics/IPhysBody.h"
#include "../shared/ADM/Physics/PhysBody.h"
#include "../shared/ADM/Physics/PhysBody_Box.h"
#include "../shared/ADM/Physics/PhysicsWorld.h"

#include "PhysBox.h"

LINK_ENTITY_TO_CLASS( phys_box, CPhysBox );

void CPhysBox::Spawn()
{
	pev->movetype = MOVETYPE_PUSH;
	pev->solid = SOLID_BSP;

	if ( pev->model )
	{
		SET_MODEL( ENT( pev ), STRING( pev->model ) );
	}

	SetThink( &CPhysBox::PhysSpawnThink );
	pev->nextthink = gpGlobals->time + 0.05f;
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

void CPhysBox::PhysThink()
{
	if ( pev->nextthink > gpGlobals->time )
		return;

	// TO-DO:
	// Instead of directly setting the origin,
	// we could calculate and set pev->velocity
	// and that way, we'll avoid clipping into players
	// Same story for angles, I suppose?
	pev->origin = PhysUtils::mtou( physBody.GetOrigin() );
	pev->angles = physBody.GetAngles();

	pev->nextthink = gpGlobals->time + (1.f / 64.f);
	
	//ALERT( at_console, "CPhysBox::PhysThink time %3.2f\n", pev->nextthink );
}

void CPhysBox::PhysSpawnThink()
{
	if ( g_Physics.IsLoaded() )
	{
		physBody.Init( this, params );

		SetThink( &CPhysBox::PhysThink );

		// For some odd reason, brush entities think 
		// in weird intervals and don't respect pev->nextthink
		// so we'll force FL_ALWAYSTHINK and manage thinking by ourselves
		pev->flags |= FL_ALWAYSTHINK;
	}

	pev->nextthink = gpGlobals->time + (1.f / 64.f);
	
	//ALERT( at_console, "CPhysBox::PhysSpawnThink time %3.2f\n", pev->nextthink );
}

void CPhysBox::Touch( CBaseEntity* other )
{
	// Apply some force if the player is pushing
	if ( !other->IsPlayer() )
		return;

	// This ain't exactly correct
	// We should just be putting the player's velocity
	// as a force, but considering the player can get stuck...
	// this may push it out of the player
	Vector delta = pev->origin - other->pev->origin;
	Vector force = delta.Normalize() * (1.0f / delta.Length());

	force = force * 1000.f;

	btVector3 btDelta( delta.x, delta.z, delta.y );
	btVector3 btForce( force.x, force.z, force.y );
	
	btDelta = -btDelta;
	//btDelta = -PhysUtils::utom(btDelta);
	//btForce = PhysUtils::utom(btForce);

	physBody.GetRigidBody()->activate( true );
	physBody.GetRigidBody()->applyImpulse( btForce, btDelta );
	//physBody.GetRigidBody()->applyForce( btForce, btDelta );

	ALERT( at_console, "Force: %3.2f\n", force.Length() );
}
