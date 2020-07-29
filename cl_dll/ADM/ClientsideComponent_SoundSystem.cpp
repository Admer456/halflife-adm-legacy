#include "HUD/hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"
#include "../shared/ADM/DelayLoad.h"

#include "Sound/SoundSystemCore.h"
#include "Sound/BaseSound.h"
#include "Sound/Channel.h"
#include "SDL2/SDL.h"

#include "ADM/System/SDLWrapper.h"

using namespace AdmSound;

void FMODTest()
{
	g_SoundSystem->PlaySound( "adm/sound/fmodtest.mp3", Channel_Music );
}

void CClientFMOD::InitExtension()
{
	SoundSystem* soundSystem = new SoundSystem();

	// If it were a non-pointer, it'd crash
	soundSystem->Init();
	soundSystem->SetupChannelGroups();

	soundSystem->LoadSound( BaseSound( "adm/sound/fmodtest.mp3" ) );

	gEngfuncs.pfnAddCommand( "sound_fmod_test", &FMODTest );
}

void CClientFMOD::Think()
{
	bool isMinimised = System::IsWindowMinimised() || !System::IsWindowInFocus();
	bool isPaused = System::GetPausedMode();

	g_SoundSystem->Update( isPaused, isMinimised );
}


