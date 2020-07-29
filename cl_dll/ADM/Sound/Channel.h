#pragma once

namespace AdmSound
{
	class ChannelGroup;
	
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
	
		void AddChannel( Channel* channel )
		{
			auto FMODChannel = channel->GetFMODChannel();
			FMODChannel->setChannelGroup( group );
		}

		void AddGroup( ChannelGroup* channelGroup )
		{
			group->addGroup( channelGroup->GetFMODGroup() );
		}

		void SetPaused( bool paused )
		{
			group->setPaused( paused );
		}

		void SetVolume( float volume )
		{
			group->setVolume( volume );
		}

		int GetNumChannels()
		{
			int number = 0;
			
			group->getNumChannels( &number );
			
			return number;
		}

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