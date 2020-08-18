#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "Player.h"

#include "AudioShared.h"
#include "AudioSound2D.h"

void AudioSound2D::Spawn()
{
	ALERT( at_console, "AudioSound2D::Spawn\n" );
}

void AudioSound2D::OnPlayerJoin( CBasePlayer* player )
{
	ALERT( at_console, "AudioSound2d::OnPlayerJoin\n" );

	CBaseEntity::OnPlayerJoin( player );

	MESSAGE_BEGIN( MSG_ONE, gmsgSound2DViaString, pev->origin, player->pev );
	
	WRITE_BYTE( channelNumber );
	WRITE_BYTE( volume * 255 );
	WRITE_SHORT( entindex() );
	WRITE_STRING( STRING( soundName ) );
	
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
