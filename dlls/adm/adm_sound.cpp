/*

======= adm_sound.cpp ===========================

	3rd February 2019
	FMOD Implementation

|||||||||||||||||||||||||||||||||||||||||||||||||||

	1st March 2019
	MY GOD, is this a mess. I'll have to 1) fix the
	issues and 2) refactor the code, oh my GOD!!!
	- Admer

|||||||||||||||||||||||||||||||||||||||||||||||||||

	8th August 2019
	We're back. It's time to rock and roll.

	From tutorials:
	https://web.archive.org/web/20080915192146/http://hlpp.thewavelength.net/tuts/mp3.htm
	https://www.gamedev.net/articles/programming/general-and-gameplay-programming/a-quick-guide-to-fmod-r2098/

	Planned format support: MP3, OGG, high-quality WAV, MID, S3M etc.

*/
#ifndef CLIENT_DLL
#include "extdll.h"
//#include "cbase.h"
#include "util.h"
#endif

#ifdef CLIENT_DLL
#include "hud.h"
#include "cl_util.h"
#endif

// TO-DO
// 0. Initial todos
// Fix compiler errors, C2011, C2027, C2084 (done)
// God damn headers (done)
// Fix error LNK2019 (done)
//
// 1. The correct way of implementing FMOD - the foundation
// Rewrite this crap
// Make server-client messages so that clients can play back sounds
// Migrate to FMDO Studio 1.10.15 (cur ver is 1.10.10 IIRC)
// Intercept PRECACHE_SOUND() and sound playback functions to FMOD
//
// 2. Nice little features
// Sound events (dynamic audio design)
// DSP effects
// Reading sound waves, if possible

#include "adm_sound.h"
#include "../FMOD/inc/fmod.hpp"
#include "../FMOD/inc/fmod_common.h"
#include "../FMOD/inc/fmod_errors.h"

#include <string>

// Some parts of this aren't actually my code, or rather my algorithms
// Some credit goes to TheTinySteini

// The sound interface is clientside only, while the sound entities are serverside only.
// It works as sound entities send messages to the clients in the PAS, and they play it back.

// Having talked to Sniper a bit about SC's FMOD implementation, I figured that's a pretty
// good way to do it. However, this would also mean that we gotta replace the sound precache
// macro, as well as sound playback stuff. Nonetheless, it is all very possible. :) (Admer)

void FMOD_ERRCHECK(FMOD_RESULT result)
{
#ifndef CLIENT_DLL
	if (result != FMOD_OK)
	{
		ALERT(at_error, "Server FMOD\n%s\n", FMOD_ErrorString(result));
	}

	else
		ALERT(at_aiconsole, "Server FMOD: All OK at this step\n");
#endif

#ifdef CLIENT_DLL
	if (result != FMOD_OK)
	{
		gEngfuncs.Con_Printf("Client FMOD\n%s\n", FMOD_ErrorString(result));
	}

	else
		gEngfuncs.Con_Printf("Client FMOD: All OK at this step\n");
#endif
}

int GetCurrentDir(char *szBuffer, int length)
{
	int bytes = GetModuleFileName(NULL, szBuffer, length);
	if (bytes == 0)
		return -1;
	else
		return bytes;
}

unsigned short int SoundTable::AddString( std::string string )
{
	vStrings.push_back( string );

	return 0;
}

std::string SoundTable::GetString( int index )
{
	return vStrings.at( index );

	std::string szNull = "null";

	return szNull;
}
