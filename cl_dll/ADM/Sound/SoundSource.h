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

		virtual void	Play( bool fromStart = false ) = 0;
		virtual void	Update() = 0;
		virtual void	Pause( bool stop = false ) = 0;
		virtual bool	IsPlaying() = 0;
	
		virtual int		GetSoundFlags() = 0;
		virtual void	SetSoundFlags( const int& flags ) = 0;
	};

	class SoundSource : public ISoundSource
	{
	public:
		SoundSource( const char* soundPath );
		virtual	~SoundSource() = default;

		virtual void	Play( bool fromStart = false ) override;
		virtual void	Update() override;
		virtual void	Pause( bool stop = false ) override;
		bool			IsPlaying() override;

		int				GetSoundFlags() override;
		void			SetSoundFlags( const int& flags ) override;

		unsigned short	entityOwner{ 1 << 15 }; // Meant to be used for entity tracking; 1 << 15 means no owner

	protected:
		FMOD::System*	system{ nullptr };
		FMOD::Channel*	channel{ nullptr };
		BaseSound*		sound{ nullptr };
		SoundState		state;

		float			soundDuration{ -1.0f };
		float			startedPlaying{ -1.0f };
	};
}