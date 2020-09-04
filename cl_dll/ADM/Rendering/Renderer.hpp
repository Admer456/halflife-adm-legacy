#pragma once

#include "GL/glew.h"

struct SDL_Window;

// Later on we'll do some sorta interface so we can have different renderers just in case lol
// Let's keep it super simple for now
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
	GLuint defaultProgram; // A simple shader that is used to render a default triangle

	float clearRed{ 0.2 };
	float clearGreen{ 0.2 };
	float clearBlue{ 0.2 };

private:
	void InitDefaultModel();
	void RenderDefaultModel();
};

extern Renderer gRenderer;
