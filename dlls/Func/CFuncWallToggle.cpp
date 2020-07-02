#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "CBaseDoor.h"
#include "CFuncWall.h"
#include "CFuncWallToggle.h"

#define SF_WALL_START_OFF		0x0001

LINK_ENTITY_TO_CLASS( func_wall_toggle, CFuncWallToggle );

void CFuncWallToggle::Spawn( void )
{
	CFuncWall::Spawn();
	if ( pev->spawnflags & SF_WALL_START_OFF )
		TurnOff();
}

void CFuncWallToggle::TurnOff( void )
{
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
	UTIL_SetOrigin( pev, pev->origin );
}


void CFuncWallToggle::TurnOn( void )
{
	pev->solid = SOLID_BSP;
	pev->effects &= ~EF_NODRAW;
	UTIL_SetOrigin( pev, pev->origin );
}


BOOL CFuncWallToggle::IsOn( void )
{
	if ( pev->solid == SOLID_NOT )
		return FALSE;
	return TRUE;
}


void CFuncWallToggle::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int status = IsOn();

	if ( ShouldToggle( useType, status ) )
	{
		if ( status )
			TurnOff();
		else
			TurnOn();
	}
}