#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "Player.h"

#include "AudioShared.h"
#include "AudioSound2D.h"

LINK_ENTITY_TO_CLASS( audio_sound2d, AudioSound2D );

void AudioSound2D::Spawn()
{
	ALERT( at_console, "AudioSound2D::Spawn\n" );
}

void AudioSound2D::OnPlayerJoin( CBasePlayer* player )
{
	ALERT( at_console, "AudioSound2D::OnPlayerJoin\n" );

	CBaseEntity::OnPlayerJoin( player );

	MESSAGE_BEGIN( MSG_ONE, gmsgSound2DViaString, pev->origin, player->pev );
	
	WRITE_BYTE( channelNumber );
	WRITE_STRING( STRING( soundName ) );
	WRITE_BYTE( volume * 255 );
	WRITE_BYTE( pev->spawnflags );
	WRITE_SHORT( entindex() );
	
	MESSAGE_END();
}

void AudioSound2D::KeyValue( KeyValueData* pkvd )
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
	else
	{
		KeyvaluesFromBase( CBaseEntity );
	}
}

void AudioSound2D::Use( CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value )
{
	MESSAGE_BEGIN( MSG_ALL, gmsgSoundManipulate );

	WRITE_BYTE( Sound_Play );
	WRITE_SHORT( entindex() );

	MESSAGE_END();
}

void AudioSound2D::Think()
{

}
