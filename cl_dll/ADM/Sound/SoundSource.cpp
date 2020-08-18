#include "SoundSystemCore.h"
#include "BaseSound.h"
#include "Channel.h"
#include "SoundSource.h"

#include "WRect.h"
#include "CL_DLL.h"

using namespace AdmSound;

SoundSource::SoundSource( const char* soundPath, unsigned int soundFlags, unsigned int channelType )
{
	// Retrieve the sound we're looking for 
	sound = g_SoundSystem->GetSound( soundPath );
	system = g_SoundSystem->GetFMODSystem();

	// The sound is stopped at first
	state = SoundState::Stopped;
	flags = soundFlags;

	soundDuration = sound->GetSoundDuration();

	// Initialise the channel
	system->playSound( *sound, nullptr, true, &channel );

	// Add the sound into the array
	g_SoundSystem->RegisterSound( this );

	// If it does not start silent, play it straight away
	if ( !(soundFlags & SoundSource_StartSilent) )
	{
		Play( true );
	}

	// If force-looped, then make it looped regardless of cue points
	if ( soundFlags & SoundSource_ForceLooped )
	{
		SetLooped( true );
	}

	// Set it to an appropriate channel group
	if ( channelType )
	{

	}
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

bool SoundSource::GetLooped()
{
	FMOD_MODE mode = 0;
	channel->getMode( &mode );
	
	bool isNotLooped = mode & FMOD_LOOP_OFF;
	return !isNotLooped;
}

void SoundSource::SetLooped( bool loop )
{
	channel->setMode( loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF );
}

int SoundSource::GetSoundFlags()
{
	return flags;
}

void SoundSource::ProcessEvent( uint16_t eventType )
{
	switch ( eventType )
	{
	case Sound_Play:
		Play( true );
		break;

	case Sound_Pause:
		Pause();
		break;

	case Sound_Unpause:
		Play( false );
		break;

	case Sound_Stop:
		Pause( true );
		break;

	case Sound_FadeVolume:
	case Sound_ChangeVolume:
	case Sound_ChangeRadius:
	case Sound_ChangePosition:
		gEngfuncs.Con_Printf( "NOTE: This sound event type is not yet implemented!\n" );
		break;
	}
}

void SoundSource::SetSoundFlags( const int& flags )
{
	channel->setMode( flags );
}
