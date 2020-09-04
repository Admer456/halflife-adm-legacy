#pragma once

// This is required so that we can link against a static GLEW lib
// instead of relying on the DLL, which seems to not be compatible with delay-loading
// This way, we avoid having to load it from the Half-Life root directory and we avoid
// having to use a DLL/SO at all
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

#include "GL/glew.h"

struct SDL_Window;

// Later on we'll do some sorta interface so we can have different renderers just in case lol
// Let's keep it super simple for now.
// Currently, the main menu rendering is broken, precisely the fonts. 
class Renderer final
{
public:
	enum GLProfile : uint32_t
	{
		Core = 0x001,
		Compatibility = 0x002,
		ES = 0x004
	};

	struct GLMode
	{
		uint32_t major;
		uint32_t minor;
		uint32_t profile;
	};

	static void SetOpenGLMode( int major, int minor, GLProfile mode );
	static GLMode GetOpenGLMode();
	static void PrintCurrentOpenGL();

public:
	Renderer() = default;
	virtual ~Renderer() = default;

	// Initialises the OpenGL 3.3 thingy whatever
	void Init();

	// Renders a frame (all visible models, applies shaders etc.)
	// Called from HUD_DrawNormalTriangles
	void RenderFrame();

	// Sets the clear colours
	void SetClear( float r, float g, float b );

	// Reloads all shaders (only the default one for now
	void ReloadShaders( bool firstTime = true );

	// Clears and deletes everything
	void Shutdown();

private:
	SDL_Window* window;
	SDL_GLContext context;

	GLuint testVertArray; // This is a VAO
	GLuint testVertBuf; // This is a VBO
	GLuint testElemBuf; // This is an EBO
	GLuint defaultProgram; // A simple shader that is used to render a default triangle
	GLuint uniformTime;

	float clearRed{ 0.2 };
	float clearGreen{ 0.2 };
	float clearBlue{ 0.2 };

	bool madeTestTriangle{ false };

private:
	// Creates a triangle
	void InitDefaultModel();

	// Renders the triangle
	void RenderDefaultModel();
};

extern Renderer gRenderer;
