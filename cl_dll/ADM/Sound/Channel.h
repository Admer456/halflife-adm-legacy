#pragma once

namespace AdmSound
{
	class ChannelGroup;
	class ISoundSource;
	
	class Channel
	{
	public:
		Channel() = default;
		~Channel() = default;

		FMOD::Channel* GetFMODChannel() { return channel; }

	public:
		operator FMOD::Channel*() const
		{
			return channel;
		}

		operator FMOD::Channel**()
		{
			return &channel;
		}

	private:
		FMOD::Channel* channel;
	};

	/*
		A channel group is a group of channels
		If you set the volume of a group, you're setting it
		to all channels in the group

		The sound system currently has only 4 channel groups,
		master, game, music and dialogue
	*/
	class ChannelGroup
	{
	public:
		ChannelGroup() = default;
		~ChannelGroup() = default;
	
		void AddChannel( Channel* channel );
		void AddSoundSource( ISoundSource* soundSource );
		void AddGroup( ChannelGroup* channelGroup );
		void SetPaused( bool paused );
		void SetVolume( float volume );
		int GetNumChannels();

		FMOD::ChannelGroup* GetFMODGroup() { return group; }

	public:
		operator FMOD::ChannelGroup*() const
		{
			return group;
		}

		operator FMOD::ChannelGroup**()
		{
			return &group;
		}

	private:
		FMOD::ChannelGroup* group;
	};
}