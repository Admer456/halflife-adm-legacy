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

		char*			GetFullPath() 
		{ 
			char fullPath[_MAX_PATH];
			const char* modName = gEngfuncs.pfnGetGameDirectory();

			sprintf( fullPath, "%s/%s", modName, path );

			return fullPath;
		}

		float			GetSoundDuration() // in seconds
		{
			unsigned int durationMilliseconds;
			sound->getLength( &durationMilliseconds, FMOD_TIMEUNIT_MS );

			return durationMilliseconds / 1000.0f;
		}

		unsigned int	GetSoundDurationMs() // in milliseconds
		{
			unsigned int durationMilliseconds;
			sound->getLength( &durationMilliseconds, FMOD_TIMEUNIT_MS );

			return durationMilliseconds;
		}

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
		unsigned short	ID;
	};
}