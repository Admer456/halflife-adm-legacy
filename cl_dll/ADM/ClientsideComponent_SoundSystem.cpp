#include "HUD/hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"
#include "../shared/ADM/DelayLoad.h"

#include "Sound/SoundSystemCore.h"
#include "Sound/BaseSound.h"
#include "Sound/Channel.h"
#include "SDL2/SDL.h"

#include "Sound/SoundSource.h"
#include "Sound/SoundSystemCore.h"

#include "ADM/System/SDLWrapper.h"

using namespace AdmSound;

void FMODTest()
{
	g_SoundSystem->PlaySound( "adm/sound/fmodtest.mp3", Channel_Music );
}

/*
=======================================================
	CClientFMOD::InitExtension
	Starts the sound system
=======================================================
*/
void CClientFMOD::InitExtension()
{
	SoundSystem* soundSystem = new SoundSystem();

	// If it were a non-pointer, it'd crash
	soundSystem->Init();
	soundSystem->SetupChannelGroups();

	soundSystem->LoadSound( BaseSound( "adm/sound/fmodtest.mp3" ) );

	gEngfuncs.pfnAddCommand( "sound_fmod_test", &FMODTest );
}

/*
=======================================================
	CClientFMOD::Think
	Checks if the game is paused etc., and updates
	the sound system
=======================================================
*/
void CClientFMOD::Think()
{
	bool isMinimised = System::IsWindowMinimised() || !System::IsWindowInFocus();
	bool isPaused = System::GetPausedMode();

	g_SoundSystem->Update( isPaused, isMinimised );
}

int CClientFMOD::MsgFunc_SndP2Str( const char* pszName, int iSize, void* pbuf )
{
	char fullRelativePath[260];
	char gameDir[32];

	ChannelType channelNumber;
	char* fileName = nullptr;
	float volume;
	uint32_t flags;
	uint16_t entityIndex;

	SoundSource* soundSource = nullptr;

	BEGIN_READ( pbuf, iSize );

	channelNumber = static_cast<ChannelType>( READ_BYTE() );
	fileName = READ_STRING();
	volume = READ_BYTE() / 255.f;
	flags = READ_BYTE();
	entityIndex = READ_SHORT();

	UTIL_GetGameDir( gameDir, 32 );

	sprintf( fullRelativePath, "%s/%s", gameDir, fileName );

	// Allocate a new sound source - will be automatically
	// added to the global sound system and tracked by it
	soundSource = new SoundSource( fileName, flags );
	soundSource->entityOwner = entityIndex;

	return 1;
}

int CClientFMOD::MsgFunc_SndP3Str( const char* pszName, int iSize, void* pbuf )
{
	BEGIN_READ( pbuf, iSize );

	Vector position;

	position.x = READ_COORD();
	position.y = READ_COORD();
	position.z = READ_COORD();

	ChannelType channelNumber = static_cast<ChannelType>( READ_BYTE() );

	char* fileName = READ_STRING();

	g_SoundSystem->PlaySound( fileName, channelNumber );

	return 1;
}

/*
	TODO:
	Implement these other functions.
*/
int CClientFMOD::MsgFunc_SndP2Num( const char* pszName, int iSize, void* pbuf )
{
	BEGIN_READ( pbuf, iSize );

	READ_BYTE();

	READ_SHORT();

	return 1;
}

int CClientFMOD::MsgFunc_SndP3Num( const char* pszName, int iSize, void* pbuf )
{
	BEGIN_READ( pbuf, iSize );

	READ_COORD();
	READ_COORD();
	READ_COORD();

	READ_BYTE();

	READ_SHORT();


	return 1;
}

int CClientFMOD::MsgFunc_SndCStr( const char* pszName, int iSize, void* pbuf )
{
	BEGIN_READ( pbuf, iSize );

	READ_STRING();

	READ_SHORT();

	return 1;
}

int CClientFMOD::MsgFunc_SndManip( const char* pszName, int iSize, void* pbuf )
{
	BEGIN_READ( pbuf, iSize );

	unsigned int mode = READ_BYTE();
	unsigned int entIndex = READ_SHORT();

	g_SoundSystem->SendSoundEvent( entIndex, mode );

	return 1;
}
