/*

======= adm_control.cpp ==============================

	Control API
	Hopefully this is going to work.
	The main purpose of this is to enable holding keys.
	Basically, you have an unsigned short int in regular
	Half-Life. It has 16 bits in total, and that's all
	you're going to get. The purpose of this, therefore,
	is to enable having more than that, using a separate
	interface. Hopefully, this implementation will be
	mostly clientside. We don't want someone's key press
	to affect every player on the server, haha.

	The process starts in input.cpp, where the player
	presses a key, thus the state of a key is down.
	That will send a signal to the Control API via a
	function that will look for any matching custom
	key strings inside of it.

	Each custom key can do one of 2 things:
	 1. Pass the string to client command
	 2. Make the Control API perform stuff

	Passing the string to client command is easy.
	Making the control API perform stuff is... a bit harder.
	It splits into two types of control: local player and global control.

	Local player control is a bit messy, but, it will
	work well. Being serverside, it'll take up some
	bandwidth. :( Hmm. Maybe I could check for something...

*/


/*
#include "adm_control.h"
#include "APIProxy.h"

#ifndef EngineClientCmd
	#define EngineClientCmd (*gEngfuncs.pfnClientCmd)
#endif

extern cl_enginefunc_t gEngfuncs;

void AdmControlAPI::ApplyControl(char *szControl)
{

	EngineClientCmd(szControl);
}

void AdmControlAPI::OnButtonUp(char *szControl)
{
	int iIndexMax = szKeyList.size;
	iIndex = 0;

	for (iIndex = 0; iIndex < iIndexMax; iIndex++)
	{
		if (strcmp(szKeyList[iIndex], szControl))
		{
			bitKeyList[iIndex] = false;
		}
	}
}

void AdmControlAPI::OnButtonDown(char *szControl)
{
	int iIndexMax = szKeyList.size;
	iIndex = 0;

	for (iIndex = 0; iIndex < iIndexMax; iIndex++)
	{
		if (strcmp(szKeyList[iIndex], szControl))
		{
			bitKeyList[iIndex] = true;
		}
	}
}
 */
// Abandoned it when I noticed EngineClientCmd xd