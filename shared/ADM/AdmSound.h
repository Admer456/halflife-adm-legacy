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

#include "fmod.hpp"
#include <vector>

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