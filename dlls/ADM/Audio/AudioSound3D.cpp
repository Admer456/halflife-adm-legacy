#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "Player.h"

#include "AudioShared.h"
#include "AudioSound3D.h"

LINK_ENTITY_TO_CLASS( audio_sound3d, AudioSound3D );

TYPEDESCRIPTION AudioSound3D::m_SaveData[] = 
{
	DEFINE_FIELD( AudioSound3D, soundName, FIELD_STRING ),
	DEFINE_FIELD( AudioSound3D, volume, FIELD_FLOAT ),
	DEFINE_FIELD( AudioSound3D, channelNumber, FIELD_INTEGER ),
	DEFINE_FIELD( AudioSound3D, customSoundLocation, FIELD_VECTOR )
};

IMPLEMENT_SAVERESTORE( AudioSound3D, CBaseEntity );

void AudioSound3D::Spawn()
{
	// Force this entity to be transmitted even tho' it has no model and whatnot
	// Currently, it isn't useful, but once sound position tracking gets implemented,
	// then this will prove veeery valuable
	SetNetworkPlowMode( ForcePlow_Everything );
}

void AudioSound3D::OnPlayerJoin( CBasePlayer* player )
{

	CBaseEntity::OnPlayerJoin( player );

	MESSAGE_BEGIN( MSG_ONE, gmsgSound3DViaString, pev->origin, player->pev );

	WRITE_BYTE( channelNumber );
	WRITE_STRING( STRING( soundName ) );
	WRITE_BYTE( volume * 255 );
	WRITE_BYTE( pev->spawnflags );
	WRITE_SHORT( entindex() );

	if ( customSoundLocation == g_vecZero )
	{
		WRITE_LONG( pev->origin.x );
		WRITE_LONG( pev->origin.y );
		WRITE_LONG( pev->origin.z );
	}
	else
	{
		WRITE_LONG( customSoundLocation.x );
		WRITE_LONG( customSoundLocation.y );
		WRITE_LONG( customSoundLocation.z );
	}

	MESSAGE_END();
}

void AudioSound3D::KeyValue( KeyValueData* pkvd )
{
	if ( KeyvalueToken( soundName ) )
	{
		KeyvalueToString( soundName );
	}
	else if ( KeyvalueToken( volume ) )
	{
		KeyvalueToFloat( volume );
	}
	else if ( KeyvalueToken( channelNumber ) )
	{
		KeyvalueToInt( channelNumber );
	}
	else if ( KeyvalueToken( customSoundLocation ) )
	{
		KeyvalueToVector( customSoundLocation );
	}
	else
	{
		KeyvaluesFromBase( CBaseEntity );
	}
}

void AudioSound3D::Use( CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value )
{
	MESSAGE_BEGIN( MSG_ALL, gmsgSoundManipulate );

	WRITE_BYTE( Sound_Play );
	WRITE_SHORT( entindex() );

	MESSAGE_END();
}

void AudioSound3D::Think()
{

}
