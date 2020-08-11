#pragma once

/*
=======================================================
	SoundSource - a base class for all sound sources

	This is the class you use to spawn 2D sounds in 
	the client's world and play them. It's mainly
	to be used for UI sounds and stuff like that.
=======================================================
*/
namespace AdmSound
{
	enum class SoundState
	{
		Stopped,
		Paused,
		Playing
	};

	class ISoundSource
	{
	public:
		ISoundSource() = default;
		~ISoundSource() = default;

		void			Play( bool fromStart = false );
	};

	class SoundSource
	{
	public:
		SoundSource( const char* soundPath );
		virtual	~SoundSource() = default;

		void			Play( bool fromStart = false );
		void			Update();
		void			Pause( bool stop = false );

		unsigned short	entityOwner{ -1 }; // -1 means no owner

	protected:
		FMOD::System*	system{ nullptr };
		BaseSound*		sound{ nullptr };
		FMOD::Channel*	channel{ nullptr };
		SoundState		state;

		float			soundDuration{ -1.0f };
		float			startedPlaying{ -1.0f };
	};
}