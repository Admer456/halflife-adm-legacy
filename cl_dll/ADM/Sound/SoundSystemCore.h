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
		void			Shutdown();

		void			SetupChannelGroups();

		void			Update( bool paused = false, bool windowMinimised = false );
		float			GetTime();

		void			LoadSound( BaseSound& sound, int flags = FMOD_DEFAULT );

		BaseSound*		GetSound( const char* soundPath );
		BaseSound*		GetSound( unsigned short soundID );

		void			PlaySound( const char* soundPath, ChannelType channel );
		void			PlaySound( unsigned short soundID, ChannelType channel );
		void			PlaySound( BaseSound& sound, ChannelType channel );

		void			RegisterSound( ISoundSource* soundSource );

		void			PrintNumChannels();

		FMOD::System*	GetFMODSystem() { return system; }

		const char*		PathFromID( unsigned short ID );
		unsigned short	IDFromPath( const char* path );

		constexpr static int MaxSoundChannels = 128;

	private:
		FMOD::System*	system = nullptr;
		void*			extraDriverData = nullptr;

		std::vector<BaseSound> sounds;

		std::vector<SoundIDPair> soundIDPairs;

		/*
			There are 3 main channel groups:
			Game sounds: default sounds.
			Music
			Voice: NPC dialogue
		*/
		ChannelGroup*	master = nullptr;
		ChannelGroup	channelGroups[4]; // Main channel group categories

		ChannelGroup	channels[Channel_Max]; // Channel groups themselves
		FMOD::Channel*	defaultChannel = nullptr; // Do not play in this channel, reserved for some testing stuff

		ISoundSource*	soundSources[MaxSoundChannels]; // These are the things from which we'll actually play the sound from

		//HMODULE			fmodLibrary;
	};

	extern SoundSystem* g_SoundSystem;

	static_assert( Channel_Max <= 32, "There can be no more than 32 channels" );

	void ErrorCheck( FMOD_RESULT result );
}

//extern AdmSound::SoundSystem* g_SoundSystem;
