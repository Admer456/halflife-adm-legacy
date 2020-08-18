#pragma once

namespace AdmSound
{
	class SoundIDPair;

	// Basic 2D sound that plays once
	class BaseSound
	{
	public:
		BaseSound() = default;
		~BaseSound() = default;

		BaseSound( unsigned short soundID )
			: ID( soundID )
		{
			path = g_SoundSystem->PathFromID( soundID );
		}

		BaseSound( const char* soundPath )
			: path( soundPath )//, ID(soundID)
		{
			ID = g_SoundSystem->IDFromPath( soundPath );
		}

		FMOD::Sound*	GetFMODSound() const { return sound; }
		unsigned short	GetID() { return ID; }

		const char*		GetPath()
		{
			return path;
		}

		char*			GetFullPath();
		float			GetSoundDuration(); // in seconds
		unsigned int	GetSoundDurationMs(); // in milliseconds

	public:
		operator FMOD::Sound*() const
		{
			return sound;
		}

		operator FMOD::Sound**()
		{
			return &sound;
		}

	protected:
		FMOD::Sound*	sound = nullptr;	// Done by SoundSystem::LoadSound
		const char*		path = nullptr;
		char			fullPath[_MAX_PATH];
		unsigned short	ID;
	};
}