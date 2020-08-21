#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "UtilServerCommand.h"

LINK_ENTITY_TO_CLASS( util_servercommand, CUtilServerCommand );

TYPEDESCRIPTION CUtilServerCommand::m_SaveData[] =
{
	DEFINE_FIELD( CUtilServerCommand, command, FIELD_STRING )
};

IMPLEMENT_SAVERESTORE( CUtilServerCommand, CBaseEntity );

void CUtilServerCommand::KeyValue( KeyValueData* pkvd )
{
	if ( KeyvalueToken( command ) )
	{
		KeyvalueToString( command );
	}
	else
		KeyvaluesFromBase( CBaseEntity );
}

void CUtilServerCommand::Use( CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value )
{
	if ( command == -1 || !STRING( command )[0] )
	{
		ALERT( at_error, "Command string invalid ('%s')\n", STRING( pev->targetname ) );
		return;
	}

	SERVER_COMMAND( (char*)STRING(command) );
}
