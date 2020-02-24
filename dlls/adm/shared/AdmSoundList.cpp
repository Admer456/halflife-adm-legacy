#include "extdll.h"
#include "util.h"

#include "AdmSoundList.h"

std::vector< std::string > g_ClientSoundList;
std::vector< std::string > g_ServerSoundList;

int PrecacheSoundClient( char* string )
{
	for ( unsigned int i = 0; i < g_ClientSoundList.size(); i++ )
	{
		if ( g_ClientSoundList.at( i ) == string )
		{
			return i; // sound was already precached, let's return it its position
		}
	}

	g_ClientSoundList.push_back( string );
	g_engfuncs.pfnPrecacheSound( string ); // no duplicates were found, let's actually precache the sound
	return g_ClientSoundList.size() - 1;
}

int PrecacheSoundServer( char* string )
{
	for ( unsigned int i = 0; i < g_ServerSoundList.size(); i++ )
	{
		if ( g_ServerSoundList.at( i ) == string )
		{
			return i; // sound was already precached, let's return it its position
		}
	}

	g_ServerSoundList.push_back( string );
	g_engfuncs.pfnPrecacheSound( string ); // no duplicates were found, let's actually precache the sound
	return g_ServerSoundList.size() - 1;
}

int FindPrecachedSound( std::string string )
{
	for ( unsigned int i = 0; i < g_ServerSoundList.size(); i++ )
	{
		if ( g_ServerSoundList.at( i ) == string )
			return i;
	}

	return 0; // could not find sound
}

void ClearSoundListClient()
{
	g_ClientSoundList.clear();
}

void ClearSoundListServer()
{
	g_ServerSoundList.clear();
}
