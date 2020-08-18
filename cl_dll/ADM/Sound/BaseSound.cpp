#include "WRect.h"
#include "CL_DLL.h"

#include "SoundSystemCore.h"
#include "BaseSound.h"
#include "Channel.h"

namespace AdmSound
{
	char* BaseSound::GetFullPath()
	{
		const char* modName = gEngfuncs.pfnGetGameDirectory();

		sprintf( fullPath, "%s/sound/%s", modName, path );

		return fullPath;
	}

	float BaseSound::GetSoundDuration()
	{
		unsigned int durationMilliseconds;
		sound->getLength( &durationMilliseconds, FMOD_TIMEUNIT_MS );

		return durationMilliseconds / 1000.0f;
	}

	unsigned int BaseSound::GetSoundDurationMs()
	{
		unsigned int durationMilliseconds;
		sound->getLength( &durationMilliseconds, FMOD_TIMEUNIT_MS );

		return durationMilliseconds;
	}
}