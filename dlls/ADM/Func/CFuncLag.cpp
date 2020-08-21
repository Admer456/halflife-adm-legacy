#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "CFuncLag.h"

#include <chrono>
using namespace std::chrono;

LINK_ENTITY_TO_CLASS( func_lag, CFuncLag );

// force optimisations so we get equal results in debug and release builds
#pragma optimize("s", on)
#pragma optimize("g", on)

void CFuncLag::Use( CBaseEntity *pActivator, CBaseEntity *pOther, USE_TYPE useType, float value )
{
	TraceResult tr;
	auto lagStart = std::chrono::system_clock::now();

	for ( int height = 0; height < 512; height++ )
	{
		for ( int width = 0; width < 512; width++ )
		{
			UTIL_TraceLine( pev->origin, pev->origin + Vector( 1024, width - 256, height - 256 ), ignore_monsters, ENT( pev ), &tr );
		}
	}

	auto lagEnd = std::chrono::system_clock::now();
	duration<float> lagTime = lagEnd - lagStart;

	float framerateCost = 60.0 - (1 / (0.016 + lagTime.count()));
	ALERT( at_console, "\nLag successful, with 262'144 tracelines at a cost of %f seconds, costing us %f frames per second if we were at 60fps\n", lagTime.count(), framerateCost );
}