#include "WRect.h"
#include "CL_DLL.h"

#include "SoundSystemCore.h"
#include "BaseSound.h"
#include "Channel.h"
#include "SoundSource.h"

using namespace AdmSound;

void ChannelGroup::AddSoundSource( ISoundSource* soundSource )
{
	auto FMODChannel = soundSource->GetChannel();

	FMODChannel->setChannelGroup( group );
}

int ChannelGroup::GetNumChannels()
{
	int number = 0;

	group->getNumChannels( &number );

	return number;
}

void ChannelGroup::AddChannel( Channel* channel )
{
	auto FMODChannel = channel->GetFMODChannel();
	FMODChannel->setChannelGroup( group );
}

void ChannelGroup::AddGroup( ChannelGroup* channelGroup )
{
	group->addGroup( channelGroup->GetFMODGroup() );
}

void ChannelGroup::SetPaused( bool paused )
{
	group->setPaused( paused );
}

void ChannelGroup::SetVolume( float volume )
{
	group->setVolume( volume );
}
