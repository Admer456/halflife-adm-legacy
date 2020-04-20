#include "HUD/hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"

// Thank God that Valve decided to
// port HL to Linux & Mac so we can use this
#include "SDL2/SDL.h"
#include <SDL2/SDL_mouse.h>
#include "SDL2/SDL_video.h"

// I do apologise for the global variables here
// This branch isn't quite meant to be merged into the main one anyway
SDL_Window*		mainWindow = nullptr;
SDL_Window*		window = nullptr;
SDL_Renderer*	renderer = nullptr;
SDL_Surface*	surface = nullptr;
bool			windowOn = false;

extern Vector	v_origin;
extern Vector	v_angles;
Vector			lastSimOrg;

void CClientRCCar::InitExtension()
{
	// This is an ugly hack, but we gotta get the main window SOMEHOW
	for ( Uint32 i = 0; i < 10; i++ )
	{
		mainWindow = SDL_GetWindowFromID( i );
		if ( mainWindow )
			break;
	}

	lastSimOrg = Vector( 0, 0, 0 );
}

void CClientRCCar::Think()
{
	if ( !windowOn || window == nullptr || surface == nullptr )
		return;

	PollEvents();

	// draw some crap here, lol
	SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
	SDL_RenderClear( renderer );

	SDL_SetRenderDrawColor( renderer, 255, 255, 255, 32 );
	SDL_RenderDrawLine( renderer, 64, 256, 512 - 64, 256 );
	SDL_RenderDrawLine( renderer, 256, 64, 256, 512 - 64 );

	SDL_SetRenderDrawColor( renderer, 255, 160, 0, 255 );

	int px = 256, py = 256;
	Vector forward, right, up;
	AngleVectors( v_angles, forward, right, up );
	Vector delta = v_origin - lastSimOrg;
	delta.z = 0;
	float dotForward, dotRight;

	dotForward = DotProduct( delta, forward );
	dotRight = DotProduct( delta, right );

	px += dotRight*10.0; py -= dotForward*10.0;

	SDL_Rect rect;
	rect.x = px - 15;
	rect.w = 30;
	rect.y = py - 15;
	rect.h = 30;

	SDL_RenderDrawRect( renderer, &rect );

	SDL_RenderPresent( renderer );
	SDL_UpdateWindowSurface( window );

	lastSimOrg = v_origin;
}

void Event_MouseDown( SDL_Event e )
{
	int mx, my;

	SDL_GetMouseState( &mx, &my );

	gEngfuncs.Con_Printf( "owo %i %i \n", mx, my );
}

void CClientRCCar::PollEvents()
{
	SDL_PumpEvents();

	SDL_Event e;
	while ( SDL_PollEvent( &e ) )
	{
		switch ( e.type )
		{
		case SDL_MOUSEBUTTONDOWN:
			Event_MouseDown( e );
			break;
		}
	}
}

void CClientRCCar::OpenWindow()
{
	window = SDL_CreateWindow( "Test: extra windows",
							   SDL_WINDOWPOS_UNDEFINED,
							   SDL_WINDOWPOS_UNDEFINED,
							   512, 512, SDL_WINDOW_SHOWN );
	
	SDL_RaiseWindow( window );

	surface = SDL_GetWindowSurface( window );
	renderer = SDL_CreateRenderer( window, -1, SDL_RendererFlags::SDL_RENDERER_SOFTWARE );

	windowOn = true;
}

void CClientRCCar::CloseWindow()
{
	SDL_RaiseWindow( mainWindow );

	surface = nullptr;
	SDL_DestroyWindow( window );
	window = nullptr;

	windowOn = false;
}