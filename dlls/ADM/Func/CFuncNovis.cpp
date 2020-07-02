#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "CFuncNovis.h"

LINK_ENTITY_TO_CLASS( func_novis, CFuncNovis );

void CFuncNovis::Spawn()
{
	pev->classname = ALLOC_STRING( "func_novis" );
	pev->effects = EF_NODRAW;
	pev->solid = SOLID_BSP;
	pev->flags = FL_WORLDBRUSH;
	SET_MODEL( ENT( pev ), STRING( pev->model ) );    // set size and link into world
	pev->movetype = MOVETYPE_PUSHSTEP;
}