/*

======= adm_physics.cpp ===========================

	30th January 2019
	TriAPI Wrapper

|||||||||||||||||||||||||||||||||||||||||||||||||||

	Simplified rendering wrapper for now.
	Will be able to be used for many things. :3

*/

#include "hud.h"
#include "adm_renderer.h"

void ADM_RendCull(TRICULLSTYLE cull)
{
	gEngfuncs.pTriAPI->CullFace(cull);
}

void ADM_RendMode(int mode) 
{
	gEngfuncs.pTriAPI->RenderMode(mode);
}

void ADM_RendBegin(int RendererMode, int RenderMode, TRICULLSTYLE Cull) 
{
	ADM_RendMode(RenderMode);
	ADM_RendCull(Cull);
	gEngfuncs.pTriAPI->Begin(RendererMode);
}

void ADM_RendEnd()
{
	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
}

void ADM_RendSetVert(float XPercent, float YPercent) 
{
	float x = static_cast<float>ScreenWidth  * (XPercent / 100);
	float y = static_cast<float>ScreenHeight * (YPercent / 100);

	gEngfuncs.pTriAPI->TexCoord2f(XPercent / 100, YPercent / 100);
	gEngfuncs.pTriAPI->Vertex3f(x, y, 0);
}

void ADM_RendSetVert(float x3d, float y3d, float z3d) 
{
	gEngfuncs.pTriAPI->TexCoord2f(x3d, y3d);
	gEngfuncs.pTriAPI->Vertex3f(x3d, y3d, z3d);
}

void ADM_RendSetVert(vec3_t position) 
{ 
	ADM_RendSetVert(position.x, position.y, position.z); 
}

void ADM_RendSetBrightness(float brightness) 
{
	gEngfuncs.pTriAPI->Brightness(brightness);
}

int  ADM_RendGetSprite(const char* SpritePath, int frame) 
{
	return gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)gEngfuncs.GetSpritePointer( SPR_Load(SpritePath) ), frame);
}

int  ADM_RendWTS(vec3_t *world, vec3_t *screen) 
{
	return gEngfuncs.pTriAPI->WorldToScreen(*world, *screen);
}

void ADM_RendSTW(float *screen, float *world) 
{
	gEngfuncs.pTriAPI->ScreenToWorld(screen, world);
}

// Some utilities

Vector GetHUDColours()
{
	return Vector(CVAR_GET_FLOAT("adm_hud_r"), CVAR_GET_FLOAT("adm_hud_g"), CVAR_GET_FLOAT("adm_hud_b"));
}

/*
	float < 1.6 -> 4:3 (0)
	float == 1.6 -> 16:10 a.k.a 8:5 (1)
	float > 1.6 -> 16:9 (2)
*/
int GetAspectRatio()
{
	float ratio = static_cast<float>(ScreenWidth / ScreenHeight);

	if (ratio < 1.6)
		return 0; // 4:3

	else if (ratio == 1.6)
		return 1; // 8:5

	else if (ratio > 1.6)
		return 2; // 16:9
}

// Renderer base

void ADM_RendDrawLine(vec3_t a, vec3_t b)
{
	ADM_RendBegin(TRI_LINES);

	a.z = 0;
	b.z = 0;

	ADM_RendSetVert(a);
	ADM_RendSetVert(b);

	ADM_RendEnd();
}

void ADM_RendDrawLine(float x1, float y1, float x2, float y2)
{
	ADM_RendGetSprite("sprites/white32.spr", 0);
	ADM_RendBegin(TRI_LINES);

	gEngfuncs.pTriAPI->Color4f(255, 0, 0, 255);

	x1 *= ScreenWidth;
	x2 *= ScreenWidth;
	y1 *= ScreenHeight;
	y2 *= ScreenHeight;

	ADM_RendSetVert(x1, y1, 0);
	ADM_RendSetVert(x2, y2, 0);

	ADM_RendEnd();
}

// AdmSrc HUD System

/*

*/