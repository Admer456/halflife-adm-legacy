#include "WRect.h"
#include "CL_DLL.h"
#include "../shared/ADM/DelayLoad.h"

#include "SoundSystemCore.h"
#include "BaseSound.h"
#include "Channel.h"
#include "SoundSource.h"
#include "SoundSourceSpatial.h"


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
#ifdef WIN32
	DelayLoad_LoadGameLib( "fmod.dll", "cl_dlls" );
#endif

	// Start the FMOD sound system
	ErrorCheck( FMOD::System_Create( &system ), "Created the FMOD system" );
	ErrorCheck( system->init( 128, FMOD_INIT_NORMAL, extraDriverData ), "Initialised the FMOD system" );

	// Set up the 3D listener settings
	ErrorCheck( system->set3DSettings( DopplerScale, DistanceFactor, RolloffScale ), "Listener settings initialised" );

	// Enforce that this is a singleton
	if ( g_SoundSystem == nullptr )
	{
		g_SoundSystem = this;
	}
	else
	{
		gEngfuncs.Con_Printf( "ERROR: Tried to instantiate SoundSystem more than once!" );
		assert( !"Tried to instantiate SoundSystem more than once" );
	}

	memset( soundSources, 0, sizeof( ISoundSource* )*MaxSoundChannels*2 );
}

/*
=======================================================
	SoundSystem::Reset
	Clears all sounds that should be cleared
	as well as sound sources
=======================================================
*/
void SoundSystem::Reset()
{
	if ( !g_SoundSystem )
		return;

	// TODO: Don't stop actively playing music?
	// Stop all sounds
	master->SetPaused( true );
	
	for ( auto& source : soundSources )
	{
		if ( !source )
			continue;

		source->Pause( true );
		source->GetChannel()->setChannelGroup( nullptr );

		// Delete all sound sources
		delete source;
		source = nullptr;
	}

	// Release all sounds
	for ( auto& sound : sounds )
	{
		auto fmodSound = sound.GetFMODSound();
		sound.GetFMODSound()->release();
	}

	sounds.clear();

	// But do not shut down the system yet
}

/*
=======================================================
	SoundSystem::Shutdown
	Frees all sound samples and the sound system
=======================================================
*/
void SoundSystem::Shutdown()
{
	ErrorCheck( system->release(), "Shutting down..." );
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

	//// Play a dummy sound in every channel to create that channel
	//LoadSound( BaseSound( "adm/sound/null.wav" ) );
	//for ( int i = Channel_Default; i < Channel_Max; i++ )
	//{
	//	PlaySound( "adm/sound/null.wav", static_cast<ChannelType>(i) );
	//}

	// Set the channels to their respective groups
	for ( int i = Channel_Default; i < Channel_Max; i++ )
	{
		if ( i >= Channel_Default && i < Channel_Music )
		{
			channelGroups[ChannelGroup_GameSounds].AddGroup( &channels[i] );
		}

		else if ( i >= Channel_Music && i < Channel_Dialogue )
		{
			channelGroups[ChannelGroup_Music].AddGroup( &channels[i] );
		}

		else if ( i >= Channel_Dialogue && i < Channel_Max )
		{
			channelGroups[ChannelGroup_Dialogue].AddGroup( &channels[i] );
		}
	}

	// Combine 3 channel groups into one master group
	master->AddGroup( &channelGroups[ChannelGroup_GameSounds] );
	master->AddGroup( &channelGroups[ChannelGroup_Music] );
	master->AddGroup( &channelGroups[ChannelGroup_Dialogue] );
}

// In FMOD, posiive X is right, positive Y is up and positive Z is forward.
//
// In GoldSRC, positive X is forward, positive Y is left, and 
// positive Z is up. It's easy to convert between these two coordinate systems.
FMOD_VECTOR AdmSound::VectorToFMODVector( const Vector& vec )
{
	FMOD_VECTOR FMODVector;
	FMODVector.z = vec.x;
	FMODVector.x = -vec.y;
	FMODVector.y = vec.z;

	return FMODVector;
}

Vector AdmSound::FMODVectorToVector( const FMOD_VECTOR& vec )
{
	Vector v;
	v.x = vec.z;
	v.y = -vec.x;
	v.z = vec.y;

	return v;
}

FMOD_VECTOR VectorToFMODVectorDirect( const Vector& vec )
{
	FMOD_VECTOR FMODVector;
	FMODVector.x = vec.x;
	FMODVector.y = vec.y;
	FMODVector.z = vec.z;

	return FMODVector;
}

// refugees from view.cpp
extern vec3_t v_origin;
extern vec3_t v_angles;
extern vec3_t v_velocity;
extern float v_frametime;

/*
=======================================================
	SoundSystem::Init
	Must be called every frame - updates the system
=======================================================
*/
void SoundSystem::Update( bool paused, bool windowMinimised )
{
	static float volume = 1.0f;
	static FMOD_VECTOR FMODPlayerPosition;
	static FMOD_VECTOR FMODPlayerVelocity;
	static FMOD_VECTOR FMODPlayerForward;
	static FMOD_VECTOR FMODPlayerUp;

	// Some player data
	Vector &playerPosition = v_origin;
	Vector playerVelocity = v_velocity;
	Vector playerAngles = v_angles;
	Vector playerForward;
	Vector playerUp;

	// Get the forward and up vector
	gEngfuncs.pfnAngleVectors( playerAngles, playerForward, nullptr, playerUp );

	// Converting these Vectors into FMOD vectors
	FMODPlayerPosition = VectorToFMODVector( playerPosition );
	FMODPlayerVelocity = VectorToFMODVector( playerVelocity );
	FMODPlayerForward = VectorToFMODVector( playerForward );
	FMODPlayerUp = VectorToFMODVector( playerUp );
	
	// Update the player's "ears"
	ErrorCheck( system->set3DListenerAttributes( 0, &FMODPlayerPosition, &FMODPlayerVelocity, &FMODPlayerForward, &FMODPlayerUp ) );

	SoundSourceSpatial* spatialSoundSource = nullptr;
	Vector spatialPos = Vector( 0, 0, 0 );

	for ( ISoundSource* soundSource : soundSources )
	{
		if ( soundSource == nullptr )
			continue;

		soundSource->Update();

		spatialSoundSource = dynamic_cast<SoundSourceSpatial*>( soundSource );
		if ( spatialSoundSource )
		{
			spatialPos = spatialSoundSource->GetPosition();
		}
	}

	system->update();

	if ( windowMinimised || paused )
	{
		volume = volume * 0.87f;
	}
	else if ( !paused )
	{
		volume = volume * 0.95f + 0.05f;
	}

	if ( paused && volume < 0.05f )
	{
		master->SetPaused( true );
	}
	else if ( !paused && volume > 0.05f )
	{
		master->SetPaused( false );
	}

	master->SetVolume( volume );
}

/*
=======================================================
	SoundSystem::GetTime
	Retrieves the current time
=======================================================
*/
float SoundSystem::GetTime()
{
	return gEngfuncs.GetClientTime();
}

/*
=======================================================
	SoundSystem::RegisterSound
	Adds a sound source to the sound source array
	so they can be updated continuously
=======================================================
*/
void SoundSystem::RegisterSound( ISoundSource* soundSource )
{
	for ( auto& source : soundSources )
	{
		if ( source == nullptr )
		{
			source = soundSource;
			return;
		}
	}
}

/*
=======================================================
	SoundSystem::AddSoundSourceToChannelGroup
	Assigns the channel of a sound source to a certain channel group
	e.g. some debris sound source will go into the Debris channel group

	This is typically called by sound source constructors,
	there is no need to call it by yourself
=======================================================
*/
void SoundSystem::AddSoundSourceToChannelGroup( ISoundSource* soundSource, uint16_t channelType )
{
	channels[channelType].AddSoundSource( soundSource );
}

/*
=======================================================
	SoundSystem::SendSoundEvent
	Finds a sound source that came from a certain 
	entity, and sends an event to it
=======================================================
*/
void SoundSystem::SendSoundEvent( uint16_t entIndex, uint16_t eventType )
{
	for ( auto& source : soundSources )
	{
		if ( source == nullptr )
			continue;

		if ( source->GetOwnerIndex() == entIndex )
		{
			source->ProcessEvent( eventType );
			return;
		}
	}
}

/*
=======================================================
	SoundSystem::LoadSound
	Creates an FMOD sound object, loads sample from disk
=======================================================
*/
void SoundSystem::LoadSound( BaseSound& sound, int flags )
{
	// Use GetFullPath, as it returns the path relative to the .exe file
	system->createSound( sound.GetFullPath(), flags, nullptr, sound );
	sounds.push_back( sound );
}

/*
=======================================================
	SoundSystem::GetSound
	Retrieves a sound from the sound list, by name
=======================================================
*/
BaseSound SoundSystem::GetSound( const char* soundPath )
{
	// For beginner programmers who don't know:
	// The & after auto is a reference
	// If I didn't put it, it'd copy the values of each sound
	// and it'd be a temporary variable, so not good for returning stuff

	for ( auto &sound : sounds )
	{
		if ( sound.GetPath() == nullptr )
			continue;

		if ( !strcmp( soundPath, sound.GetPath() ) )
		{
			return sound;
		}
	}

	// If we don't find a sound, well, let's generate one
	LoadSound( BaseSound( soundPath ) );
	return GetSound( soundPath );
}

/*
=======================================================
	SoundSystem::GetSound
	Retrieves a sound from the sound list, by ID
=======================================================
*/
BaseSound* SoundSystem::GetSound( unsigned short soundID )
{
	for ( auto &sound : sounds )
	{
		if ( soundID == sound.GetID() )
		{
			return &sound;
		}
	}

	return nullptr;
}

/*
=======================================================
	SoundSystem::PlaySound
	Plays a sound by name - use this on clientside sounds
	like UI, weapon events etc.
=======================================================
*/
void SoundSystem::PlaySound( const char* soundPath, ChannelType channel )
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
void SoundSystem::PlaySound( unsigned short soundID, ChannelType channel )
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

	// Play into the default channel
	system->playSound( FMODSound, nullptr, false, &defaultChannel );
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

#define Print gEngfuncs.Con_Printf

void SoundSystem::PrintDebugInfo()
{
	Print( "PrintDebugInfo\n" );

	for ( auto& sound : sounds )
	{
		Print( "Sound %x path %s\n", sound.GetFMODSound(), sound.GetPath() );
	}
}

/*
=======================================================
	ErrorCheck
	Checks for errors and reports them in the console
	Example: ErrorCheck( system->update() );
=======================================================
*/
void AdmSound::ErrorCheck( FMOD_RESULT result, const char* messageIfOkay )
{
	if ( result != FMOD_OK )
	{
		gEngfuncs.Con_DPrintf( "## Client FMOD: %s\n", FMOD_ErrorString( result ) );
	}

	else if ( messageIfOkay )
	{
		gEngfuncs.Con_DPrintf( "## Client FMOD: %s\n", messageIfOkay );
	}
}
