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

/*
int CAdmSoundInterface::Initialize()
{
	char fmodlib[256];
	char gamedir[256];

#ifndef CLIENT_DLL
	ALERT(at_console, "Server FMOD::Initialize()\n");

	g_engfuncs.pfnGetGameDir(gamedir);
#endif

#ifdef CLIENT_DLL
	PRINTF_Console("Client FMOD::Initialize()\n");

	sprintf(gamedir, gEngfuncs.pfnGetGameDirectory());
#endif

	iIsPlaying = 0;
	sprintf(fmodlib, "%s/adm/fmodL.dll", gamedir);
	// replace forward slashes with backslashes
	for (int i = 0; i < 256; i++)
		if (fmodlib[i] == '/') fmodlib[i] = '\\';

#ifdef CLIENT_DLL
	m_hFMod = LoadLibrary(fmodlib);
#endif

	return 1;
}
*/

/*
int CAdmSoundInterface::Load(FMOD::Sound *snd, const char *szSong)
{
#ifdef CLIENT_DLL
	gEngfuncs.Con_Printf("Client FMOD::System_Create()\n");
#endif

	result = FMOD::System_Create(&system);
	ERRCHECK(result);

#ifdef CLIENT_DLL
	PRINTF_Console("Client FMOD::system->init()\n");
#endif
	result = system->init(32, FMOD_INIT_NORMAL, extradriverdata);
	ERRCHECK(result);

#ifdef CLIENT_DLL
	gEngfuncs.Con_Printf("Client FMOD system = %d\n");
#endif

//	To-do: get this thing working for once
//	result = FMOD_Debug_Initialize(FMOD_DEBUG_LEVEL_LOG, FMOD_DEBUG_MODE_CALLBACK, callback, "" );

//	FMOD throws an error: an invalid parameter was passed to the function, or something, when calling playSound()

#ifdef CLIENT_DLL
	gEngfuncs.Con_Printf("Client FMOD::system->createSound()\n");
#endif
	result = system->createSound(szSong, FMOD_DEFAULT, 0, &snd);
	ERRCHECK(result);	

#ifdef CLIENT_DLL
	gEngfuncs.Con_Printf("Client FMOD system = %d\n");
#endif

#ifdef CLIENT_DLL
	gEngfuncs.Con_Printf("Client FMOD::sound->setMode()\n");
#endif

	result = snd->setMode(FMOD_LOOP_OFF);
	ERRCHECK(result);

#ifndef CLIENT_DLL
	ALERT(at_aiconsole, "Server FMOD system = %d\n", &system);
#endif

	return 1;
}
*/

/*
int CAdmSoundInterface::Unload(FMOD::Sound *snd)
{
	result = snd->release();
	return 1;
}
*/

/*
int CAdmSoundInterface::Play(FMOD::Sound *snd)
{
#ifdef CLIENT_DLL
	gEngfuncs.Con_Printf("Client CASI::Play()\n");
	gEngfuncs.Con_Printf("Client FMOD system = %d\n", &system);
#endif

	result = system->playSound(snd, NULL, 0, NULL);
	ERRCHECK(result);

#ifdef CLIENT_DLL
	gEngfuncs.Con_Printf("Client FMOD system = %d\n", &system);
#endif

	result = system->update();
	ERRCHECK(result);

	return 1;
}
*/

/*
int CAdmSoundInterface::Stop(FMOD::Sound *snd)
{
	iIsPlaying = 0;
	return 1;
}
*/

/*
int CAdmSoundInterface::Shutdown()
{
	
	if (m_hFMod)
	{
		result = system->close();
		result = system->release();

		FreeLibrary(m_hFMod);
		m_hFMod = NULL;
		iIsPlaying = 0;
		return 1;
	}
	else
		return 0;
}
*/

/*
int CAdmSoundInterface::Load(const char *szSong)
{
//	const char *szPath = "\0";

	result = FMOD::System_Create(&system);
	ERRCHECK(result);

	result = system->init(32, FMOD_INIT_NORMAL, extradriverdata);
	ERRCHECK(result);

	result = system->createSound(szSong, FMOD_DEFAULT, 0, &sound);
	ERRCHECK(result);

	result = sound->setMode(FMOD_LOOP_OFF);
	ERRCHECK(result);

	return 1;
}
*/

/*
int CAdmSoundInterface::Unload()
{
	result = sound->release();
	ERRCHECK(result);
	return 1;
}

int CAdmSoundInterface::Play()
{
	result = system->playSound( sound, 0, false, &channel);
	ERRCHECK(result);

	result = system->update();
	ERRCHECK(result);

	return 1;
}

int CAdmSoundInterface::Stop()
{
	iIsPlaying = 0;
	ERRCHECK(result);
	return 1;
}*/

#ifndef CLIENT_DLL

/*
class CAdmSound : public CBaseEntity
{
public:
	void Spawn();
	void Precache();
	void KeyValue(KeyValueData *pkvd);

	void SoundPlay();
	void SoundStop();
	void SoundVolume();

	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

private:
	int m_flVolume;
	char m_szPath[256];
	string_t m_iszPath;
	std::string m_strFullPath;

	FMOD::Sound *m_Sound;
	bool m_fPlaying = false;
};

LINK_ENTITY_TO_CLASS(sound_generic, CAdmSound);

// WHAT IS THIS BLACK MAGIC AAAAAAAAAAAAAAA
// THIS IS LITERALLY THE WORST CODE I'VE EVER
// WRITTEN OH MY GOD OH MY FRICKING HEAD IT 
// HURTS SO DAMN MUCH
// FUCK YOU ADMER, FUCK YOU!!!! AAAAAAAAHHHH
void CAdmSound::Spawn()
{
	char mydir[MAX_PATH];

	std::string strExeName = "hl.exe";

	GetCurrentDir(mydir, MAX_PATH);

	m_strFullPath = mydir;
	m_strFullPath.erase((m_strFullPath.length() - strExeName.length()), strExeName.length());

	ALERT(at_console, "CAdmSound::Spawn()\n");
	

	sprintf(m_szPath, "%s", m_strFullPath.c_str());
	
	std::string strFullPath = m_szPath;
	strFullPath += "adm\\";
	strFullPath += STRING(m_iszPath);

	for (int i = 0; i < strFullPath.length(); i++)
		if (strFullPath[i] == '/') strFullPath[i] = '\\';

	m_strFullPath = strFullPath;

	sprintf(m_szPath, "%s", m_strFullPath.c_str());
	m_iszPath = ALLOC_STRING(m_szPath);

	ALERT(at_console, "Current working directory:\n%s \n", m_strFullPath.c_str());

	Precache();
}

void CAdmSound::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "path"))
	{
		m_iszPath = ALLOC_STRING(pkvd->szValue);
		sprintf(m_szPath, "%s", pkvd->szValue);
		
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "volume"))
	{
		m_flVolume = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else
		CBaseEntity::KeyValue(pkvd);

//	Precache();
}

void CAdmSound::Precache()
{
	ALERT(at_aiconsole, "CAdmSound::SoundPrecache()\n");
	ALERT(at_aiconsole, "File: %s \n", m_strFullPath.c_str());

//	gAdmSound.Load(m_Sound, m_strFullPath.c_str());
}

void CAdmSound::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	ALERT(at_console, "CAdmSound::Use()\n");

	if (!m_fPlaying)
	{
		SoundPlay();
		m_fPlaying = true;
	}
	
	else if (m_fPlaying)
	{
		SoundStop();
		m_fPlaying = false;
	}
}

void CAdmSound::SoundPlay()
{
	char dir[256];
	g_engfuncs.pfnGetGameDir(dir);

	ALERT(at_aiconsole, "CAdmSound::SoundPlay()\n");
	ALERT(at_aiconsole, "File: \n %s \n", m_strFullPath.c_str());

//	gAdmSound.Play(m_Sound);
}

void CAdmSound::SoundStop()
{
	ALERT(at_aiconsole, "CAdmSound::SoundStop()\n");
//	gAdmSound.Stop(m_Sound);
}

void CAdmSound::SoundVolume()
{
	ALERT(at_aiconsole, "CAdmSound::SoundVolume()\n");
	m_Sound->setMusicChannelVolume(0, m_flVolume);
}
*/
#endif // !CLIENT_DLL

