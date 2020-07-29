#include "SoundSystemCore.h"
#include "BaseSound.h"
#include "Channel.h"

#include "WRect.h"
#include "CL_DLL.h"
#include "../shared/ADM/DelayLoad.h"

#include <cassert>

namespace AdmSound
{
	SoundSystem* g_SoundSystem = nullptr;
}

using namespace AdmSound;

/*
=======================================================
	SoundSystem::Init
	Starts the sound system
=======================================================
*/
void SoundSystem::Init()
{
	DelayLoad_LoadGameLib( "fmod.dll", "cl_dlls" );

	ErrorCheck( FMOD::System_Create( &system ) );
	ErrorCheck( system->init( Channel_Max, FMOD_INIT_NORMAL, extraDriverData ) );

	if ( g_SoundSystem == nullptr )
	{
		g_SoundSystem = this;
	}
	else
	{
		gEngfuncs.Con_Printf( "ERROR: Tried to instantiate SoundSystem more than once!" );
		assert( !"Tried to instantiate SoundSystem more than once" );
	}
}

/*
=======================================================
	SoundSystem::Shutdown
	Frees all sound samples and the sound system
=======================================================
*/
void SoundSystem::Shutdown()
{
	ErrorCheck( system->release() );
}

/*
=======================================================
	SoundSystem::SetupChannelGroups
	Sets up channel groups and initialises all sound channels
=======================================================
*/
void SoundSystem::SetupChannelGroups()
{
	// Grab the master channel and allocate the channel groups
	system->getMasterChannelGroup(			channelGroups[ChannelGroup_Master] );
	system->createChannelGroup( "Game",		channelGroups[ChannelGroup_GameSounds] );
	system->createChannelGroup( "Music",	channelGroups[ChannelGroup_Music] );
	system->createChannelGroup( "Dialogue", channelGroups[ChannelGroup_Dialogue] );

	master = &channelGroups[ChannelGroup_Master];

	// Play a dummy sound in every channel to create that channel
	LoadSound( BaseSound( "adm/sound/null.wav" ) );
	for ( int i = Channel_Default; i < Channel_Max; i++ )
	{
		PlaySound( "adm/sound/null.wav", static_cast<ChannelType>(i) );
	}

	// Set the channels to their respective groups
	for ( int i = Channel_Default; i < Channel_Max; i++ )
	{
		FMOD::Channel* channel = channels[i].GetFMODChannel();

		if ( i >= Channel_Default && i < Channel_Music )
		{
			channel->setChannelGroup( channelGroups[ChannelGroup_GameSounds] );
		}

		else if ( i >= Channel_Music && i < Channel_Dialogue )
		{
			channel->setChannelGroup( channelGroups[ChannelGroup_Music] );
		}

		else if ( i >= Channel_Dialogue && i < Channel_Max )
		{
			channel->setChannelGroup( channelGroups[ChannelGroup_Dialogue] );
		}
	}

	// Combine 3 channel groups into one master group
	master->AddGroup( &channelGroups[ChannelGroup_GameSounds] );
	master->AddGroup( &channelGroups[ChannelGroup_Music] );
	master->AddGroup( &channelGroups[ChannelGroup_Dialogue] );
}

/*
=======================================================
	SoundSystem::Init
	Must be called every frame - updates the system
=======================================================
*/
void SoundSystem::Update( bool paused, bool windowMinimised )
{
	system->update();

	master->SetPaused( paused );

	if ( windowMinimised )
	{
		master->SetVolume( 0.0f );
	}
	else if ( !paused )
	{
		master->SetVolume( 1.0f );
	}
}

/*
=======================================================
	SoundSystem::LoadSound
	Creates an FMOD sound object, loads sample from disk
=======================================================
*/
void SoundSystem::LoadSound( BaseSound& sound )
{
	system->createSound( sound.GetPath(), FMOD_DEFAULT, nullptr, sound );
	sounds.push_back( sound ); 
}

/*
=======================================================
	SoundSystem::PlaySound
	Plays a sound by name - use this on clientside sounds
	like UI, weapon events etc.
=======================================================
*/
void AdmSound::SoundSystem::PlaySound( const char* soundPath, ChannelType channel )
{
	for ( auto sound : sounds )
	{
		if ( sound.GetPath() == nullptr )
			continue;

		if ( !strcmp( soundPath, sound.GetPath() ) )
		{
			PlaySound( sound, channel );
			return;
		}
	}
}

/*
=======================================================
	SoundSystem::PlaySound
	Plays a sound by ID - used by the serverside
	and is less expensive than playing by name
=======================================================
*/
void AdmSound::SoundSystem::PlaySound( unsigned short soundID, ChannelType channel )
{
	for ( auto sound : sounds )
	{
		if ( soundID == sound.GetID() )
		{
			PlaySound( sound, channel );
			break;
		}
	}
}

/*
=======================================================
	SoundSystem::PlaySound
	Plays a sound by object - used internally
=======================================================
*/
void SoundSystem::PlaySound( BaseSound& sound, ChannelType channel )
{
	auto FMODSound = sound.GetFMODSound();
	
	if ( FMODSound == nullptr )
	{
		LoadSound( sound );
	}

	system->playSound( FMODSound, nullptr, false, channels[channel] );
}

/*
=======================================================
	SoundSystem::PrintNumChannels
	Debugging
	Prints the number of channels in each channel group
=======================================================
*/
void SoundSystem::PrintNumChannels()
{
	gEngfuncs.Con_Printf( "--- PrintNumChannels ---\n" );
	gEngfuncs.Con_Printf( "Total channels: %i\n",		Channel_Max );
	gEngfuncs.Con_Printf( "Game sound channels: %i\n",	channelGroups[ChannelGroup_GameSounds].GetNumChannels() );
	gEngfuncs.Con_Printf( "Music channels: %i\n",		channelGroups[ChannelGroup_Music].GetNumChannels() );
	gEngfuncs.Con_Printf( "Dialogue channels: %i\n",	channelGroups[ChannelGroup_Dialogue].GetNumChannels() );
	gEngfuncs.Con_Printf( "\n" );
}

/*
=======================================================
	SoundSystem::PathFromID
	Returns a sound name from the ID
	Generally used for fast handling of sounds
	that we received from the serverside
=======================================================
*/
const char* SoundSystem::PathFromID( unsigned short ID )
{
	for ( auto pair : soundIDPairs )
	{
		if ( ID == pair.GetID() )
		{
			return pair.GetPath();
		}
	}

	return "null";
}

/*
=======================================================
	SoundSystem::IDFromPath
	Returns a sound ID from name
	Generally used to resolve sound names coming from
	the serverside
=======================================================
*/
unsigned short SoundSystem::IDFromPath( const char* path )
{
	for ( auto pair : soundIDPairs )
	{
		if ( !strcmp( path, pair.GetPath() ) )
		{
			return pair.GetID();
		}
	}

	return -1;
}

/*
=======================================================
	ErrorCheck
	Checks for errors and reports them in the console
	Example: ErrorCheck( system->update() );
=======================================================
*/
void AdmSound::ErrorCheck( FMOD_RESULT result )
{
	if ( result != FMOD_OK )
	{
		gEngfuncs.Con_DPrintf( "Client FMOD\n%s\n", FMOD_ErrorString( result ) );
	}

	else
	{
		gEngfuncs.Con_DPrintf( "Client FMOD: All OK at this step\n" );
	}
}
