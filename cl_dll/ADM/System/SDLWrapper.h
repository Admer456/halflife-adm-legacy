#pragma once

namespace System
{
	SDL_Window*	GetMainGameWindow();
	bool IsWindowInFocus( SDL_Window* window = nullptr );
	bool IsWindowMinimised( SDL_Window* window = nullptr );
	void SetPausedMode( bool paused );
	bool GetPausedMode();
}
