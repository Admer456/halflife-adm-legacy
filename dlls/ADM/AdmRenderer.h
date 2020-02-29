/*

======= adm_renderer.h ===========================
	________________________
	30th January 2019
	________________________
	AdmSrc TriAPI Wrapper
	________________________
	Header file
	________________________

*/

#pragma once

#include "cl_dll.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "util_vector.h"

#include "triangleapi.h"
#include "tri.h"

// Set Cull
// TRI_FRONT 0
// TRI_NONE  1
// pTriAPI->CullFace();
void ADM_RendCull(TRICULLSTYLE cull);

/*
	kRenderNormal,			// src
	kRenderTransColor,		// c*a+dest*(1-a)
	kRenderTransTexture,	// src*a+dest*(1-a)
	kRenderGlow,			// src*a+dest -- No Z buffer checks
	kRenderTransAlpha,		// src*srca+dest*(1-srca)
	kRenderTransAdd,		// src*a+dest
	Set the render mode.
	pTriAPI->RenderMode();
*/
void ADM_RendMode(int mode);

/*
TRI_TRIANGLES		0
TRI_TRIANGLE_FAN	1
TRI_QUADS			2
TRI_POLYGON			3
TRI_LINES			4	
TRI_TRIANGLE_STRIP	5
TRI_QUAD_STRIP		6
pTriAPI->Begin();*/
void ADM_RendBegin(int RendererMode, int RenderMode = kRenderTransAdd, TRICULLSTYLE Cull = TRI_NONE);

void ADM_RendEnd();

// Calls TexCoord2f and Vertex3f, basically sets a vertex
void ADM_RendSetVert(float XPercent, float YPercent); // XY in percentages, please. uwu
void ADM_RendSetVert(float x3d, float y3d, float z3d);
void ADM_RendSetVert(vec3_t position);

// Sets brightness (?)
void ADM_RendSetBrightness(float brightness);

// Gets a sprite
int ADM_RendGetSprite(const char* SpritePath, int frame);

// WorldToScreen, world is an origin coordinate
// Returns 1 if it's Z-clipped
int  ADM_RendWTS(vec3_t *world, vec3_t *screen);

// ScreenToWorld
void ADM_RendSTW(float *screen, float *world);

// More to be added

/*
	void		( *Color4f ) ( float r, float g, float b, float a );
	void		( *Color4ub ) ( unsigned char r, unsigned char g, unsigned char b, unsigned char a );

	void		( *Fog ) ( float flFogColor[3], float flStart, float flEnd, int bOn ); // Works just like GL_FOG, flFogColor is r/g/b.
	void		( *FogParams ) ( float flDensity, int iFogSkybox );

	void		( *GetMatrix ) ( const int pname, float *matrix );
	int			( *BoxInPVS ) ( float *mins, float *maxs );
	void		( *LightAtPoint ) ( float *pos, float *value );
	void		( *Color4fRendermode ) ( float r, float g, float b, float a, int rendermode );
	*/
	
// Renderer base

void ADM_RendDrawLine(vec3_t a, vec3_t b);
void ADM_RendDrawLine(float x1, float y1, float x2, float y2);

Vector GetHUDColours();
int    GetAspectRatio();

struct AdmTriApi2D
{
	void Begin(int RendererMode, int RenderMode = kRenderTransAdd, TRICULLSTYLE Cull = TRI_NONE)
	{
		ADM_RendBegin(RendererMode, RenderMode, Cull);
	}
};

struct AdmTriApiPseudo3D
{

};

struct AdmTriApi3D
{

};

// ARS - ADM Renderer Shape
// ARSB - ARS shape basic info, holds information like origin and angles

struct ARSB
{
	vec3_t vecOriginA;
	vec3_t vecOriginB;
	vec3_t vecAnglesA;
	vec3_t vecAnglesB;

	vec3_t ColourA;
	vec3_t ColourB;
	float  flPhaseOrigin; // Could be used for animations between A and B
	float  flPhaseAngles;
	float  flPhaseColour;
};

typedef Vector2D ARS_Vertex2;
typedef Vector   ARS_Vertex3;

// Triangle

struct ARS_Tri2
{
	ARSB base;
	ARS_Vertex2 A;
	ARS_Vertex2 B;
	ARS_Vertex2 C;
};

struct ARS_Tri3
{
	ARSB base;
	ARS_Vertex3 A;
	ARS_Vertex3 B;
	ARS_Vertex3 C;
};

// Square

struct ARS_Square
{
	ARSB base;
	float x;
};

// Rectangles

struct ARS_Rect2
{
	ARSB base;
	ARS_Vertex2 A;
	ARS_Vertex2 B;
};

struct ARS_Rect3
{
	ARSB base;
	ARS_Vertex3 A;
	ARS_Vertex3 B;
};

// Quadrilaterals

struct ARS_Quad2
{
	ARSB base;
	ARS_Vertex2 A;
	ARS_Vertex2 B;
	ARS_Vertex2 C;
	ARS_Vertex2 D;
};

struct ARS_Quad3
{
	ARSB base;
	ARS_Vertex3 A;
	ARS_Vertex3 B;
	ARS_Vertex3 C;
	ARS_Vertex3 D;
};

// HUD system and HUD elements

class CAdmHudRenderer
{
public:
	void Init();
	

	AdmTriApi2D R2D;
};

class CAdmHudFrame
{
public:
	void Init();
	void Draw();

	int index;
	int depth;

	int iShading; // 0 - no shading, 1 - Windows-like shading, 2 - diagonal-split shading
	int iFilling; // 0 - no fill, 1 - diagonal fill, 2 - full fill, 3 - grid fill

/*
desc:

	A frame.

	It can be shaded in 3 ways:
	- Windows-like shading
	- Diagonal-split shading
	- no shading

	It can be outlined, and has ways of filling stuff:
	- diagonal fill
	- full fill
	- grid fill


*/

private:
	ARS_Rect2 Frame;
};
