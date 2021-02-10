//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "HUD/hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "Exports.h"
#include "com_model.h"

#include "particleman.h"
#include "tri.h"
extern IParticleMan *g_pParticleMan;

#include "ADM/AdmRenderer.h"
#include "ADM/Components/SimpleSky.h"

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles( void )
{
//	RecClDrawNormalTriangles();

	gHUD.m_Spectator.DrawOverview();
}

#if defined( _TFC )
void RunEventList( void );
#endif

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles( void )
{
//	RecClDrawTransparentTriangles();

	triangleapi_t* triAPI = gEngfuncs.pTriAPI;

#if defined( _TFC )
	RunEventList();
#endif

	//gHUD.m_clImgui.TestDrawImgui();
	//gHUD.m_clImgui.DisplayImgui();

	g_Sky.Render( triAPI );

	if ( g_pParticleMan )
		 g_pParticleMan->Update();
}

/*
=================
HUD_DrawOrthoTriangles
Orthogonal Triangles -- (relative to resolution,
smackdab on the screen) add them here
=================
*/

void HUD_DrawOrthoTriangles(void)
{

}
