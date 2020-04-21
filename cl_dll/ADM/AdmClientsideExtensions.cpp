/*

======= adm_clientside_extensions.cpp ========================

	8th August 2019
	Clientside extensions system.

	Basically, the client receives messages from the server.
	However, it's all handled through the HUD.
	I wanted to make a more separate thing that is reserved more
	for general clientside things, not just HUD things.

	Think of it as a piggyback ride off the HUD system.
*/

#include "HUD/hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"

int CBaseClientExtension::Init( void )
{
	gHUD.AddHudElem( this );
	m_iFlags |= HUD_ACTIVE; // always active

	InitExtension();

	return 1;
}

/*
List of extensions:

CClientFMOD m_clFMOD;

*/

void CHud::InitExtensions( void )
{
	m_clFMOD.Init();
	m_clPunch.Init();
	m_clImgui.Init();
}

void CHud::VidInitExtensions( void )
{
	m_clFMOD.VidInit();
}

