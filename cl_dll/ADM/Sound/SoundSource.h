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
	#include "ADM/Audio/AudioShared.h"

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

		virtual int		GetOwnerIndex() = 0;
		virtual void	ProcessEvent( uint16_t eventType ) = 0;

		virtual FMOD::Channel* GetChannel() = 0;
	};

	class SoundSource : public ISoundSource
	{
	public:
		SoundSource( const char* soundPath, unsigned int soundFlags = 0, unsigned int channelType = 0 );
		virtual	~SoundSource() = default;

		virtual void	Play( bool fromStart = false ) override;
		virtual void	Update() override;
		virtual void	Pause( bool stop = false ) override;
		bool			IsPlaying() override;

		int				GetSoundFlags() override;
		void			SetSoundFlags( const int& flags ) override;

		int				GetOwnerIndex() override { return entityOwner; }
		void			ProcessEvent( uint16_t eventType ) override;
		
		FMOD::Channel*	GetChannel() override { return channel; }

		bool			GetLooped();
		void			SetLooped( bool loop );

		unsigned short	entityOwner{ 1 << 15 }; // Meant to be used for entity tracking; 1 << 15 means no owner

	protected:
		FMOD::System*	system{ nullptr };
		FMOD::Channel*	channel{ nullptr };
		BaseSound		sound;
		SoundState		state;

		unsigned int	flags{ 0 };
		float			soundDuration{ -1.0f };
		float			startedPlaying{ -1.0f };
	};
}