#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "CBaseDoor.h"
#include "CFuncWall.h"
#include "CFuncMonsterClip.h"

LINK_ENTITY_TO_CLASS( func_monsterclip, CFuncMonsterClip );

void CFuncMonsterClip::Spawn( void )
{
	CFuncWall::Spawn();
	if ( CVAR_GET_FLOAT( "showtriggers" ) == 0 )
		pev->effects = EF_NODRAW;
	pev->flags |= FL_MONSTERCLIP;
}
