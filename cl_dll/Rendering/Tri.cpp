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

#include "SDL2/SDL.h"
#include "ADM/System/SDLWrapper.h"
#include "ADM/Rendering/Renderer.hpp"

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

	if ( !System::GetPausedMode() )
	{
		gRenderer.RenderFrame();

		g_Sky.Render( triAPI );
	}

	if ( g_pParticleMan )
		 g_pParticleMan->Update();
}

//#define ORTHOEXAMPLE
//#ifdef ORTHOEXAMPLE
void OrthoExample() // AdmSrc - this ain't my code, lol
{
	gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd); //additive

	// use hotglow, or any other sprite for the texture
	gEngfuncs.pTriAPI->SpriteTexture((struct model_s *)
		gEngfuncs.GetSpritePointer(SPR_Load("sprites/hotglow.spr")),
		0);

	gEngfuncs.pTriAPI->CullFace(TRI_NONE); //no culling
	gEngfuncs.pTriAPI->Begin(TRI_QUADS); //start our quad

	//remember, always list vertices in counter-clockwise 
	// order, unless you want the quad to be backwards =)
	// the third value of vertex3f will always be 0 in ortho mode, 
	// don't change it unless you wan't funny things to happen.

	//top left
	gEngfuncs.pTriAPI->TexCoord2f(0.0f, 1.0f);
	gEngfuncs.pTriAPI->Vertex3f(0, 0, 0);

	//bottom left
	gEngfuncs.pTriAPI->TexCoord2f(0.0f, 0.0f);
	gEngfuncs.pTriAPI->Vertex3f(0, ScreenHeight, 0);

	//bottom right
	gEngfuncs.pTriAPI->TexCoord2f(1.0f, 0.0f);
	gEngfuncs.pTriAPI->Vertex3f(ScreenWidth, ScreenHeight, 0);

	//top right
	gEngfuncs.pTriAPI->TexCoord2f(1.0f, 1.0f);
	gEngfuncs.pTriAPI->Vertex3f(ScreenWidth, 0, 0);

	gEngfuncs.pTriAPI->End(); //end our list of vertexes
	gEngfuncs.pTriAPI->RenderMode(kRenderNormal); //return to normal
}
//#endif

/*
=================
HUD_DrawOrthoTriangles
Orthogonal Triangles -- (relative to resolution,
smackdab on the screen) add them here
=================
*/

void HUD_DrawOrthoTriangles(void)
{
//#ifdef ORTHOEXAMPLE
//	OrthoExample();
//#endif

	ADM_RendDrawLine(0.2f, 0.9f, 0.9f, 0.9f);
	ADM_RendDrawLine(0.1f, 0.85f, 0.8f, 0.85f);
	ADM_RendDrawLine(0.2f, 0.9f, 0.1f, 0.85f);
	ADM_RendDrawLine(0.9f, 0.9f, 0.8f, 0.85f);

}
