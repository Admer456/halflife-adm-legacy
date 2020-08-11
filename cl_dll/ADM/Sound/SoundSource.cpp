#include "SoundSystemCore.h"
#include "BaseSound.h"
#include "Channel.h"
#include "SoundSource.h"

using namespace AdmSound;

SoundSource::SoundSource( const char* soundPath )
{
	sound = g_SoundSystem->GetSound( soundPath );
	system = g_SoundSystem->GetFMODSystem();

	state = SoundState::Stopped;

	soundDuration = sound->GetSoundDuration();

	system->playSound( *sound, nullptr, true, &channel );
}

void SoundSource::Play( bool fromStart )
{
	channel->setPaused( false );
	
	if ( fromStart )
	{
		channel->setPosition( 0, FMOD_TIMEUNIT_MS );
	}

	startedPlaying = g_SoundSystem->GetTime();
}

void SoundSource::Update()
{
	float endTime = startedPlaying + soundDuration;
	float currentTime = g_SoundSystem->GetTime();

	if ( currentTime > endTime )
	{
		Pause( true );
	}
}

void SoundSource::Pause( bool stop )
{
	channel->setPaused( true );

	if ( stop )
	{
		channel->setPosition( 0, FMOD_TIMEUNIT_MS );
	}
}