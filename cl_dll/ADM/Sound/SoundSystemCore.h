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

	class SoundSystem
	{
	public:
		void			Init();
		void			Shutdown();

		void			SetupChannelGroups();

		void			Update( bool paused = false, bool windowMinimised = false );

		void			LoadSound( BaseSound& sound );

		void			PlaySound( const char* soundPath, ChannelType channel );
		void			PlaySound( unsigned short soundID, ChannelType channel );
		void			PlaySound( BaseSound& sound, ChannelType channel );

		void			PrintNumChannels();

		FMOD::System*	GetFMODSystem() { return system; }

		const char*		PathFromID( unsigned short ID );
		unsigned short	IDFromPath( const char* path );

	private:
		FMOD::System*	system = nullptr;
		FMOD::Channel*	masterChannel = nullptr;
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
		ChannelGroup	channelGroups[4];
		Channel			channels[Channel_Max];

		//HMODULE			fmodLibrary;
	};

	extern SoundSystem* g_SoundSystem;

	static_assert( Channel_Max <= 64, "There can be no more than 64 FMOD channels" );

	void ErrorCheck( FMOD_RESULT result );
}

//extern AdmSound::SoundSystem* g_SoundSystem;
