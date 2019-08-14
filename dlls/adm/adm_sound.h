/*

======= adm_sound.cpp ===========================

	3rd February 2019
	FMOD Implementation

||||||||||||||||||||||||||||||||||||||||||||||||||

	From tutorials:
	https://web.archive.org/web/20080915192146/http://hlpp.thewavelength.net/tuts/mp3.htm
	https://www.gamedev.net/articles/programming/general-and-gameplay-programming/a-quick-guide-to-fmod-r2098/

	Planned format support: MP3, OGG, high-quality WAV, MIDI, S3M etc.

*/

#pragma once

#include "../FMOD/inc/fmod.hpp"
#include <vector>

struct FMODSound
{
	FMOD::System     *system;
	FMOD::Sound      *sound;
	FMOD::Channel    *channel = 0;
	FMOD_RESULT       result;
	
	void             *extradriverdata = 0;

	int				iIsPlaying;
	bool			bIsStream;
};

struct SoundTable
{
	std::vector<std::string> vStrings;
	unsigned short int iStrings = 0;

	unsigned short int AddString( std::string string );
	std::string GetString( int index );
};

#ifndef CLIENT_DLL

extern SoundTable g_ServerSoundTable;

#endif

struct FMODSoundSystem
{
	// Methods

	void Initialise();
	void Shutdown();

	void LoadSound( char *szSoundPath );
	void Play( char *szSoundPath );
	void Update();

	// Member variables
	
	FMOD::System *System;
	HMODULE m_hFmodLibrary;

	FMOD::Sound *Sound;
	FMOD::Channel *Channel = 0;

	FMOD::Sound *MusicTrack;

	FMOD_RESULT Result;
};

void FMOD_ERRCHECK(FMOD_RESULT result);

//class CAdmSoundInterface
//{
//public:
//	int	Initialize	();
//	int	Shutdown	();
//	int	Play		(FMOD::Sound *snd);
//	int	Stop		(FMOD::Sound *snd);
//	int Load		(FMOD::Sound *snd, const char *szSong);
//	int Unload		(FMOD::Sound *snd);
//
//	int	Play();
//	int	Stop();
//	int Load(const char *szSong);
//	int Unload();
//
//private:
////	FMODSound_s m_Sound;
//	HINSTANCE   m_hFMod;
//
//	FMOD::System     *system;
//	FMOD::Sound      *sound;
//	FMOD::Channel    *channel = 0;
//	FMOD_RESULT       result;
//
//	void             *extradriverdata = 0;
//	int				iIsPlaying;
//	bool			bIsStream;
//};

//extern CAdmSoundInterface gAdmSound;

//#endif