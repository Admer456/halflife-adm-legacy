#include "WRect.h"
#include "CL_DLL.h"

#include <string>
#include <sstream>
#include <fstream>
#include <istream>

#include "SDL2/SDL.h"
#include "ADM/System/SDLWrapper.h"
#include "../shared/ADM/DelayLoad.h"

#include "Renderer.hpp"

// Our default triangle
const GLfloat triangleVerts[] =
{
	-0.5f, -0.5f, 0.0f,		// bottom-left
	0.5f, -0.5f, 0.0f,		// bottom-right
	0.0f,  0.5f, 0.0f,		// top-centre
	-0.5f,  0.5f, 0.0f,		// top-left
};

const GLuint triangleIndices[] =
{
	0, 1, 3, // lower-left triangle
	1, 2, 3, // upper-right triangle
};

void PrintCurrentRenderer()
{
	return Renderer::PrintCurrentOpenGL();
}

void SetClearColour()
{
	if ( gEngfuncs.Cmd_Argc() != 4 )
	{
		gEngfuncs.Con_Printf( 
			"|| Usage: gl_ClearColour [r] [g] [b]\n"
			"|| Example: gl_ClearColour 0 0 64\n"
			"|| Parameters: r, g and b must be between 0 and 255\n"
		);

		return;
	}

	float r, g, b;

	r = atof( gEngfuncs.Cmd_Argv( 1 ) ) / 255.f;
	g = atof( gEngfuncs.Cmd_Argv( 2 ) ) / 255.f;
	b = atof( gEngfuncs.Cmd_Argv( 3 ) ) / 255.f;

	gRenderer.SetClear( r, g, b );
}

void ReloadShaders()
{
	return gRenderer.ReloadShaders( false );
}

void Renderer::SetOpenGLMode( int major, int minor, GLProfile mode )
{
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, major );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, minor );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, mode );
}

Renderer::GLMode Renderer::GetOpenGLMode()
{
	GLMode mode;
	
	int major, minor, profile;

	SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &major );
	SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, &minor );
	SDL_GL_GetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, &profile );

	mode.major = major;
	mode.minor = minor;
	mode.profile = profile;

	return mode;
}

void Renderer::PrintCurrentOpenGL()
{
	// GoldSRC uses OpenGL 2.1 legacy by default
	// The profile mask seems to be 0 (neither Core nor Compatibility), which is not surprising
	GLMode currentMode = Renderer::GetOpenGLMode();

	gEngfuncs.Con_Printf( "OpenGL info:\nVersion: %i.%i ", currentMode.major, currentMode.minor );

	// Some bleh code to print this thingy
	if ( currentMode.profile == GLProfile::Core )
	{
		gEngfuncs.Con_Printf( "Core\n" );
	}
	else if ( currentMode.profile == GLProfile::Compatibility )
	{
		gEngfuncs.Con_Printf( "Legacy\n" );
	}
	else
	{
		gEngfuncs.Con_Printf( "[unknown profile: 0x%4x]\n", currentMode.profile );
	}
}

void Renderer::Init()
{
	// Grab the game window
	window = System::GetMainGameWindow();

	// Add console commands for some stuff
	gEngfuncs.pfnAddCommand( "gl_printCurrentRenderer", PrintCurrentRenderer );
	gEngfuncs.pfnAddCommand( "gl_clearColour", SetClearColour );
	gEngfuncs.pfnAddCommand( "gl_shadersReload", ::ReloadShaders );

	// Change the OpenGL attributes to become OpenGL 3.3 Core
	Renderer::SetOpenGLMode( 3, 3, GLProfile::Core );

	// Initialise GLEW
	if ( glewInit() != GLEW_OK )
	{
		// I suppose there's a better way to... handle this? Heh
		MessageBox( nullptr, "Eror eror eror", "Glew cant load lmao", MB_OK );
		abort();
	}

	// Currently, we can't see this in the console, but we will be able to see it
	// in a debugger output or something :v
	gEngfuncs.Con_Printf( "OpenGL context: %x\n", context );

	// Load the default shader program
	ReloadShaders();

	// Find and set up a uniform for time
	uniformTime = glGetUniformLocation( defaultProgram, "time" );

	// Create a triangle so we can render it later
	InitDefaultModel();
}

void Renderer::RenderFrame()
{
	// FIRSTLY and FOREMOSTLY, we must ensure that we don't mess with the engine's rendering path
	// EPIC thanks to Overfloater who told me about these 2 lines of code, otherwise menu font
	// rendering would be broken
	// GoldSRC tracks texture binds, and if we *bind* something new, it binds that to the currently rendered texture,
	// which is almost always not our intention. We can use these 2 to avoid that
	glPushAttrib( GL_TEXTURE_BIT );
	glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );

	// The engine does a transformation matrix which we don't need
	// as it's done in the vertex shader anyway
	// glPushMatrix, glPopMatrix, glTranslate, glRotate etc. are all made obsolete
	// thanks to the magical wonders of vertex shaders
	//glPopMatrix();

	// If we want a custom clear colour, glClearColor must always be called
	// *after* the engine does it; since RenderFrame is called from HUD_DrawNormalTriangles,
	// it's in a good place. 
	//
	// By default, when onlyClientDraw is 0, the clear colour is red (if gl_clear is 1),
	// otherwise it's black (even if gl_clear is 0)
	glClearColor( clearRed, clearGreen, clearBlue, 0.01f );

	// Right now, this is not very necessary since we're just drawing a flat triangle, but
	// depending on the circumstances, we'll need it eventually
	//glDepthRange( 0.01f, 1000.0f );

	// Clear the frame
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Render the default triangle
	RenderDefaultModel();
	
	// "Unbind" this vertex array so the engine's UI renderer operates normally
	glBindVertexArray( 0 );

	// Once we're done rendering, just pop them to return back to normal
	glPopAttrib();
	glPopClientAttrib();

	// This seems to revert to a "shaderless" state of rendering
	// so the engine can render stuff normally again, almost normally
	glUseProgram( 0 );
}

void Renderer::SetClear( float r, float g, float b )
{
	clearRed = r;
	clearGreen = g;
	clearBlue = b;
}

void Renderer::ReloadShaders( bool firstTime )
{
	// Temporarily create shader objects which we can compile into
	GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
	GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

	// Always remember that you must use modfolder/shaders/Default.fp
	// not shaders/Default.fp alone, so we must get the game directory
	std::string gameDir = gEngfuncs.pfnGetGameDirectory();
	std::string vertexCode;
	std::string fragCode;
	
	std::ifstream vertexFile;
	std::ifstream fragFile;

	// SHADERS, YAY
	// Eventually, these will be combined into one file, like so:
	/*
		#shader vertex
		#version 330 core

		// some vert shader code

		#shader fragment

		// some frag shader code
	
		#shader geometry

		// some geometry shader code (optional)

	*/
	// Additionally, we'll need to support shader inclusion, so we can #include stuff

	// At the moment, these don't do any error checking while reading the file, so you may expect a crash...
	// This is in the 'test' branch family after all, so this is fine
	vertexFile.open( gameDir + "/shaders/Default.vp" );
	fragFile.open( gameDir + "/shaders/Default.fp" );

	// Putting a scope here so vertexStream and fragStream are inaccessible outside of it
	{
		std::stringstream vertexStream, fragStream;
		vertexStream << vertexFile.rdbuf();
		fragStream << fragFile.rdbuf();

		vertexFile.close();
		fragFile.close();

		vertexCode = vertexStream.str();
		fragCode = fragStream.str();
	}

	// glShaderSource does not like someString.c_str(), so we gotta provide it with a const char*
	const char* vertexRawCode = vertexCode.c_str();
	const char* fragRawCode = fragCode.c_str();

	// Compile the shaders
	glShaderSource( vertexShader, 1, &vertexRawCode, nullptr );
	glCompileShader( vertexShader );

	glShaderSource( fragmentShader, 1, &fragRawCode, nullptr );
	glCompileShader( fragmentShader );

	// If it's our first time loading (i.e. we're not *reloading* the shaders via a console command,
	// do not delete the program
	if ( !firstTime )
		glDeleteProgram( defaultProgram );
	
	// Create and link a shader program, attaching respective shaders to it
	defaultProgram = glCreateProgram();
	
	// Ultimately, attach the shaders to a program
	glAttachShader( defaultProgram, vertexShader );
	glAttachShader( defaultProgram, fragmentShader );
	glLinkProgram( defaultProgram );

	// We don't need these any more now that we compiled a shader program
	glDeleteShader( vertexShader );
	glDeleteShader( fragmentShader );
}

void Renderer::Shutdown()
{
	glDeleteProgram( defaultProgram );
}

void Renderer::InitDefaultModel()
{
	glPushAttrib( GL_TEXTURE_BIT );
	glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );

	// Create a vertex array object
	glGenVertexArrays( 1, &testVertArray );
	glBindVertexArray( testVertArray ); //<- this is the bugger

	// Generate a buffer that will contain this vertex data
	glGenBuffers( 1, &testVertBuf );
	glBindBuffer( GL_ARRAY_BUFFER, testVertBuf );
	glBufferData( GL_ARRAY_BUFFER, sizeof( triangleVerts ), triangleVerts, GL_STATIC_DRAW );	

	// Generate an EBO so we can use glDrawElements instead of glDrawArrays
	// This is a square after all
	glGenBuffers( 1, &testElemBuf );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, testElemBuf );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( triangleIndices ), triangleIndices, GL_STATIC_DRAW );

	// Set up the vertex attribute pointer for location 0
	// layout (location = 0) vec3 vPosition;
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void*)0 );
	glEnableVertexAttribArray( 0 );

	glBindVertexArray( 0 );

	glPopAttrib();
	glPopClientAttrib();
}

void Renderer::RenderDefaultModel()
{
	glDisable( GL_CULL_FACE );

	// Render the thing using the default program
	glUseProgram( defaultProgram );

	// Set a value for the "time" uniform, so we can pass some time data to the shader
	glUniform1f( uniformTime, gEngfuncs.GetClientTime() );

	glBindVertexArray( testVertArray );
	glDrawElements( GL_TRIANGLES, sizeof( triangleIndices ) / sizeof( GLuint ), GL_UNSIGNED_INT, 0 );
	//glDrawArrays( GL_TRIANGLES, 0, 3 );

	//glBindVertexArray( 0 );

	glEnable( GL_CULL_FACE );
}
