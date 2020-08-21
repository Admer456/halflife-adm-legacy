#pragma once

#include "fmod.hpp"
#include "fmod_common.h"
#include "fmod_errors.h"

#include "Channel.h"

#include <vector>
#include <string>

namespace AdmSound
{
	class BaseSound;
	class Channel;
	class ChannelGroup;
	class ISoundSource;

	// Pairs that will be later used to load the sound files
	class SoundIDPair
	{
	public:
		SoundIDPair() = default;
		~SoundIDPair() = default;

		SoundIDPair( const char* soundPath, unsigned short soundId )
			: path(soundPath), id(soundId)
		{ 
		
		}

		const char*		GetPath() { return path; }
		unsigned short	GetID() { return id; }

	private:
		const char*		path;
		unsigned short	id;
	};

	// Channel groups are used to control the 
	// parameters of multiple channels e.g. volume
	enum ChannelGroupType
	{
		ChannelGroup_Master,
		ChannelGroup_GameSounds,
		ChannelGroup_Music,
		ChannelGroup_Dialogue
	};

	// These are actually also channel groups
	// BUT they're 
	enum ChannelType
	{
		// ChannelGroup_GameSounds
		Channel_Default = 0,
		Channel_Weapons,
		Channel_Ambience,
		Channel_NPCSounds,
		Channel_Debris,
		Channel_UI,

		// ChannelGroup_Music
		Channel_Music,

		// ChannelGroup_Dialogue
		Channel_Dialogue,

		// End
		Channel_Max
	};

	// The distance factor basically expresses how many units there are in a metre
	// In this case, there is about 39.97 GoldSRC units in a metre
	constexpr float DistanceFactor = 39.970078740157480314f;

	constexpr float DopplerScale = 1.0f;
	constexpr float RolloffScale = 1.0f;

	class SoundSystem
	{
	public:
		void			Init();
		void			Reset();
		void			Shutdown();

		// Initialises all channel groups
		void			SetupChannelGroups();

		// Updates all the active sounds and deletes garbage (inactive sounds with no owners)
		void			Update( bool paused = false, bool windowMinimised = false );

		// Retrieves local client time in seconds
		float			GetTime();

		// Utility functions
		// Registers a sound in the sound system, so it can keep track of it
		void			RegisterSound( ISoundSource* soundSource );

		// Assigns the channel of a sound source to a certain channel group
		// e.g. some debris sound source will go into the Debris channel group
		void			AddSoundSourceToChannelGroup( ISoundSource* soundSource, uint16_t channelType );

		// Sends a sound event eventType to a specific sound owned by entIndex
		void			SendSoundEvent( uint16_t entIndex, uint16_t eventType );
	
		// Loads the sound from the HDD
		void			LoadSound( BaseSound& sound, int flags = FMOD_DEFAULT );

		// Utility functions to get individual sounds
		BaseSound		GetSound( const char* soundPath );
		BaseSound*		GetSound( unsigned short soundID );

		// Low-level functions that will play a sound in the default channel
		// Do not use
		void			PlaySound( const char* soundPath, ChannelType channel );
		void			PlaySound( unsigned short soundID, ChannelType channel );
		void			PlaySound( BaseSound& sound, ChannelType channel );

		// Prints the current number of channels
		void			PrintNumChannels();

		FMOD::System*	GetFMODSystem() { return system; }

		// Get sound path from a sound ID - generally used for getting sound paths from IDs that the server has sent
		const char*		PathFromID( unsigned short ID );

		// Get an ID for the given sound path
		unsigned short	IDFromPath( const char* path );

		// The maximum number of channels = maximum number of sounds playing at once
		constexpr static int MaxSoundChannels = 128;

		void			PrintDebugInfo();

	private:
		// Core system stuff
		FMOD::System*	system = nullptr;
		void*			extraDriverData = nullptr;

		// Sound handles - they contain paths an IDs for sounds, not used in other places other than this class
		std::vector<BaseSound> sounds;

		// Path and ID pairs - when the server builds a list of sound strings and IDs, it sends them
		// to the clients. Then, the server will use those IDs as handles for each sound path
		std::vector<SoundIDPair> soundIDPairs;

		/*
			There are 3 main channel groups:
			Game sounds: default sounds.
			Music
			Voice: NPC dialogue
		*/
		ChannelGroup*	master = nullptr;

		// Main channel group categories
		ChannelGroup	channelGroups[4];

		// Channel groups themselves
		ChannelGroup	channels[Channel_Max]; 

		// Do not play in this channel, reserved for some testing stuff
		FMOD::Channel*	defaultChannel = nullptr;

		// These are the things from which we'll actually play the sound from
		// BaseSound != SoundSource (!!!)
		// A sound source does not even refer to a BaseSound, it uses an FMOD sound instead
		ISoundSource*	soundSources[MaxSoundChannels*2]; 

		//HMODULE			fmodLibrary;
	};

	extern SoundSystem* g_SoundSystem;

	static_assert( Channel_Max <= 32, "There can be no more than 32 channels" );

	void ErrorCheck( FMOD_RESULT result );

	FMOD_VECTOR VectorToFMODVector( const Vector& vec );
	Vector FMODVectorToVector( const FMOD_VECTOR& vec );
}

//extern AdmSound::SoundSystem* g_SoundSystem;
