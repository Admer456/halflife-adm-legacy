/*

======= adm_clientside_sound.cpp ===========================
	
	10th August 2019
	FMOD clientside sound system

*/

// TO-DO
// 0. Initial todos
// Fix compiler errors, C2011, C2027, C2084 (done)
// God damn headers (done)
// Fix error LNK2019 (done)
//
// 1. The correct way of implementing FMOD - the foundation
// Rewrite this crap (done)
// Make server-client messages so that clients can play back sounds (done)
// Migrate to FMDO Studio 1.10.15 (cur ver is 1.10.10 IIRC)
// Intercept PRECACHE_SOUND() and sound playback functions to FMOD
//
// 2. Nice little features
// Sound events (dynamic audio design)
// DSP effects
// Reading sound waves, if possible

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"

#include <string>

#include "adm/DelayLoad.h"

// ---------------------------------
// FMOD clientside extension methods
// ---------------------------------

DECLARE_MESSAGE( m_clFMOD, FSSSound );
DECLARE_MESSAGE( m_clFMOD, FSSMusic );
DECLARE_MESSAGE( m_clFMOD, StrTable );

void CClientFMOD::InitExtension( void )
{
	HOOK_MESSAGE( FSSSound );
	HOOK_MESSAGE( FSSMusic );
	HOOK_MESSAGE( StrTable );

	gEngfuncs.Con_Printf( "\nCClientFMOD::InitExtension()\n" );

//	LoadExtendedList( gEngfuncs.pfnGetLevelName() );
	m_soundSystem.Initialise();
}

void CClientFMOD::Think( void )
{
//	UpdateSoundStatus();
	m_soundSystem.Update();
}

void CClientFMOD::LoadSound( char *szSoundPath )
{
	m_soundSystem.LoadSound( szSoundPath );
}

void CClientFMOD::PlaySound( char *szSoundPath )
{
	m_soundSystem.Play( szSoundPath );
}

int CClientFMOD::MsgFunc_FSSSound( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int iStringIndex = READ_LONG(); // string_t

	std::string szSound = GetStringFromTable( iStringIndex );

	PlaySound( (char*)szSound.c_str() );

	return 1;
}

int CClientFMOD::MsgFunc_FSSMusic( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize ); // we read two things: music path, and volume
	
	char szSoundPath[MAX_PATH];
	char *szReadPath = READ_STRING();

	UTIL_GetGameDir( szSoundPath, sizeof(szSoundPath) );
	strcat( szSoundPath, "/music/" );
	strcat( szSoundPath, szReadPath );
//	float flVolume = (float)READ_BYTE() / 255;

	PlaySound( szSoundPath );

	return 1;
}

int CClientFMOD::MsgFunc_StrTable( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	char *szString = READ_STRING();
	int iIndex = READ_LONG();

	AddToStringTable( iIndex, szString );

	return 1;
}

void CClientFMOD::AddToStringTable( int index, std::string string )
{
	m_rgIndices.push_back( index );
	m_rgStrings.push_back( string );
}

std::string CClientFMOD::GetStringFromTable( int index )
{
	for ( unsigned int i = 0; i < m_rgIndices.size(); i++ )
	{
		if ( index == m_rgIndices.at( i ) )
		{
			return m_rgStrings.at( i );
		}
	}

	std::string szNull = "null";

	return szNull;
}

// ----------------------------------
// FMOD sound system struct functions
// ----------------------------------

void FMODSoundSystem::Initialise()
{
	m_hFmodLibrary = DelayLoad_LoadGameLib( "fmod.dll", "cl_dlls" );

	Result = FMOD::System_Create( &System );
	Result = System->init( 32, FMOD_INIT_NORMAL, nullptr );

	FMOD_ERRCHECK( Result );
	gEngfuncs.Con_Printf( "\nAt Initialise()" );
}

void FMODSoundSystem::Shutdown()
{
	Result = Sound->release();
	Result = System->release();
}

void FMODSoundSystem::LoadSound( char *szSoundPath )
{
	Result = System->createSound( szSoundPath, FMOD_DEFAULT, 0, &Sound );
	FMOD_ERRCHECK( Result );
	gEngfuncs.Con_Printf( "\nAt LoadSound()" );
}

void FMODSoundSystem::Play( char *szSoundPath )
{
	LoadSound( szSoundPath );
	Result = System->playSound( Sound, 0, false, &Channel );
	FMOD_ERRCHECK( Result );
	gEngfuncs.Con_Printf( "\nAt PlaySound()" );
}

void FMODSoundSystem::Update()
{
	Result = System->update();
//	FMOD_ERRCHECK( Result );
//	gEngfuncs.Con_Printf( "\nAt Update()" );
}
