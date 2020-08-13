#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"

#include "EnvSky2D.h"

LINK_ENTITY_TO_CLASS( env_sky2d, EnvSky2D );

void EnvSky2D::Spawn()
{
	// I guess nothing for now
}

void EnvSky2D::KeyValue( KeyValueData* pkvd )
{
	if ( KeyvalueToken( rotationSpeed ) )
	{
		KeyvalueToFloat( skyParams.rotationSpeed );
	}
	else if ( KeyvalueToken( wobbleFrequency ) )
	{
		KeyvalueToFloat( skyParams.wobbleFrequency );
	}
	else if ( KeyvalueToken( wobbleScale ) )
	{
		KeyvalueToFloat( skyParams.wobbleScale );
	}
	else if ( KeyvalueToken( skyDistance ) )
	{
		KeyvalueToInt( skyParams.skyDistance );
	}
	else if ( KeyvalueToken( skyName ) )
	{
		KeyvalueToString( skyName );
	}
}

void EnvSky2D::Use( CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value )
{
	// Oofus doofus
	MESSAGE_BEGIN( MSG_ALL, gmsgSky2D );

	skyParams.skyFlags = pev->spawnflags;
	skyParams.skyName = const_cast<char*>STRING( skyName );

	WRITE_BYTE( skyParams.skyFlags );
	WRITE_SHORT( skyParams.rotationSpeed * 10.0f );
	WRITE_SHORT( skyParams.wobbleFrequency * 10.0f );
	WRITE_SHORT( skyParams.wobbleScale );
	WRITE_BYTE( skyParams.skyDistance );
	WRITE_STRING( skyParams.skyName );

	// KARA-BOOFUS!!!
	// LET THE WRATH OF GOD PUNISH THE NETWORK BANDWIDTH
	MESSAGE_END();
}
