#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "CEnvExplosion.h"
#include "CSmoker.h"

LINK_ENTITY_TO_CLASS( env_smoker, CSmoker );

void CSmoker::Spawn( void )
{
	pev->movetype = MOVETYPE_NONE;
	pev->nextthink = gpGlobals->time;
	pev->solid = SOLID_NOT;
	UTIL_SetSize( pev, g_vecZero, g_vecZero );
	pev->effects |= EF_NODRAW;
	pev->angles = g_vecZero;
}


void CSmoker::Think( void )
{
	// lots of smoke
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
	WRITE_BYTE( TE_SMOKE );
	WRITE_COORD( pev->origin.x + RANDOM_FLOAT( -pev->dmg, pev->dmg ) );
	WRITE_COORD( pev->origin.y + RANDOM_FLOAT( -pev->dmg, pev->dmg ) );
	WRITE_COORD( pev->origin.z );
	WRITE_SHORT( g_sModelIndexSmoke );
	WRITE_BYTE( RANDOM_LONG( pev->scale, pev->scale * 1.1 ) );
	WRITE_BYTE( RANDOM_LONG( 8, 14 ) ); // framerate
	MESSAGE_END();

	pev->health--;
	if ( pev->health > 0 )
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.1, 0.2 );
	else
		UTIL_Remove( this );
}
