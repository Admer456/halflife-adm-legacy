#pragma once

enum class SoundState
{
	Stopped,
	Paused,
	Playing
};

enum SoundSourceFlags
{
	// Sound doesn't immediately play
	SoundSource_StartSilent = 1 << 0,

	// Sounce loops regardless of cue points
	SoundSource_ForceLooped = 1 << 1,

	// Sound is toggled; works only if there are cue points or if it's forced looped
	SoundSource_Toggled = 1 << 2,
};

enum SoundManipulationType
{
	Sound_Play = 0,
	Sound_Pause,
	Sound_Unpause,
	Sound_Stop,
	Sound_FadeVolume,
	Sound_ChangeVolume,
	Sound_ChangeRadius,
	Sound_ChangePosition,
};

#ifndef CLIENT_DLL
extern int gmsgSound2DViaString;
extern int gmsgSound3DViaString;
extern int gmsgSoundManipulate;
#endif