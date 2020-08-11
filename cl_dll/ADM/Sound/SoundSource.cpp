#include "SoundSystemCore.h"
#include "BaseSound.h"
#include "Channel.h"
#include "SoundSource.h"

using namespace AdmSound;

SoundSource::SoundSource( const char* soundPath )
{
	// Retrieve the sound we're looking for 
	sound = g_SoundSystem->GetSound( soundPath );
	system = g_SoundSystem->GetFMODSystem();

	// The sound is stopped at first
	state = SoundState::Stopped;

	soundDuration = sound->GetSoundDuration();

	// Initialise the channel
	system->playSound( *sound, nullptr, true, &channel );

	// Add the sound into the array
	g_SoundSystem->RegisterSound( this );
}

void SoundSource::Play( bool fromStart )
{
	// Unpause the channel
	channel->setPaused( false );
	
	// Set the seeker to the beginning of the sound
	if ( fromStart )
	{
		channel->setPosition( 0, FMOD_TIMEUNIT_MS );
	}

	// Get the time when we started playin' this
	// The sound system will keep track of it and eventually
	// delete the sound if it has no owner and it hasn't been playing for ages
	startedPlaying = g_SoundSystem->GetTime();
}

void SoundSource::Update()
{
	// If it's not playing, there's no need to update it
	if ( state != SoundState::Playing )
		return;

	//float endTime = startedPlaying + soundDuration;
	//float currentTime = g_SoundSystem->GetTime();

	// Stop the sound if the actual FMOD sound reached the end
	if ( !IsPlaying() )
	{
		Pause( true );
	}
}

void SoundSource::Pause( bool stop )
{
	channel->setPaused( true );

	state = SoundState::Paused;

	if ( stop )
	{
		channel->setPosition( 0, FMOD_TIMEUNIT_MS );
	}
}

bool SoundSource::IsPlaying()
{
	bool isPlaying = true;
	channel->isPlaying( &isPlaying );

	return isPlaying;
}

int SoundSource::GetSoundFlags()
{
	return 0;
}

void SoundSource::SetSoundFlags( const int& flags )
{
	channel->setMode( flags );
}
