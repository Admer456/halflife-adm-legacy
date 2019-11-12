#pragma once

#include <vector>

// Sound table to prevent duplicate precaches
// Will be merged with the FMOD one at some point

//extern std::vector< std::string > g_ClientSoundList;
//extern std::vector< std::string > g_ServerSoundList;

int PrecacheSoundClient( char* string );
int PrecacheSoundServer( char* string );
int FindPrecachedSound( std::string string );

void ClearSoundListClient();
void ClearSoundListServer();
