#include "WRect.h"
#include "CL_DLL.h"
#include "SDL2/SDL.h"
#include "SDLWrapper.h"

namespace System
{
	SDL_Window* GetMainGameWindow()
	{
		SDL_Window* window = nullptr;

		for ( int i = 0; i < 10; i++ )
		{
			window = SDL_GetWindowFromID( i );
			if ( window )
				break;
		}

		if ( window == nullptr )
		{
			gEngfuncs.Con_Printf( "ERROR: Could not locate the game's window (HOW ON EARTH???)\n" );
			gEngfuncs.Con_Printf( "- Here's the log from SDL2, if it helps:\n%s\n", SDL_GetError() );
		}

		return window;
	}

	bool IsWindowInFocus( SDL_Window* window )
	{
		if ( window == nullptr )
			window = GetMainGameWindow();
		
		if ( window == nullptr )
			return false;

		int flags = SDL_GetWindowFlags( window );

		return flags & SDL_WINDOW_INPUT_FOCUS;
	}

	bool IsWindowMinimised( SDL_Window* window )
	{
		if ( window == nullptr )
			window = GetMainGameWindow();

		if ( window == nullptr )
			return false;

		int flags = SDL_GetWindowFlags( window );

		return flags & SDL_WINDOW_MINIMIZED;
	}

	bool localPaused = true;

	// Since there's no easy way to read whether the
	// game is paused (in the main menu) or not,
	// this function gets called from several IN_ functions
	// in InputW32.cpp
	// Might not work on Linux, which worries me
	void SetPausedMode( bool paused )
	{
		localPaused = paused;
	}

	bool GetPausedMode()
	{
		return localPaused;
	}

}
