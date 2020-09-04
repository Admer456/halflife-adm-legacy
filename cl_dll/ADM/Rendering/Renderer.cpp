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

constexpr const char* defaultVertexShader =
"#version 330 core\n"
"layout (location = 0) in vec3 vPos;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4( vPos.x, vPos.y, vPos.z, 1.0f );\n"
"}\0";

constexpr const char* defaultFragmentShader =
"#version 330 core\n"
"out vec4 cFrag;\n"
"void main()\n"
"{\n"
"	cFrag = vec4( 1.0f, 0.62f, 0.0f, 1.0f );\n"
"}\0";

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
	GLMode currentMode = Renderer::GetOpenGLMode();

	gEngfuncs.Con_Printf( "OpenGL info:\nVersion: %i.%i ", currentMode.major, currentMode.minor );

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
	window = System::GetMainGameWindow();

	gEngfuncs.pfnAddCommand( "gl_printCurrentRenderer", PrintCurrentRenderer );
	gEngfuncs.pfnAddCommand( "gl_clearColour", SetClearColour );
	gEngfuncs.pfnAddCommand( "gl_shadersReload", ::ReloadShaders );

	Renderer::SetOpenGLMode( 3, 3, GLProfile::Core );

	if ( glewInit() != GLEW_OK )
	{
		MessageBox( nullptr, "Eror eror eror", "Glew cant load lmao", MB_OK );
		abort();
	}

	glClearColor( 0.0f, 0.0f, 0.5f, 1.0f );
	glDepthRange( 0.01f, 100.0f );

	gEngfuncs.Con_Printf( "OpenGL context: %x\n", context );

	ReloadShaders();
	InitDefaultModel();

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Renderer::RenderFrame()
{
	// The engine does a transformation matrix which we don't need
	// as it's done in the vertex shader anyway
	//glPopMatrix();

	glClearColor( clearRed, clearGreen, clearBlue, 0.01f );
	glDepthRange( 0.01f, 1000.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	RenderDefaultModel();

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
	GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
	GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

	std::string gameDir = gEngfuncs.pfnGetGameDirectory();
	std::string vertexCode;
	std::string fragCode;
	
	std::ifstream vertexFile;
	std::ifstream fragFile;

	vertexFile.open( gameDir + "/shaders/Default.vp" );
	fragFile.open( gameDir + "/shaders/Default.fp" );

	{
		std::stringstream vertexStream, fragStream;
		vertexStream << vertexFile.rdbuf();
		fragStream << fragFile.rdbuf();

		vertexFile.close();
		fragFile.close();

		vertexCode = vertexStream.str();
		fragCode = fragStream.str();
	}

	const char* vertexRawCode = vertexCode.c_str();
	const char* fragRawCode = fragCode.c_str();

	// Compile the shaders
	glShaderSource( vertexShader, 1, &vertexRawCode, nullptr );
	glCompileShader( vertexShader );

	glShaderSource( fragmentShader, 1, &fragRawCode, nullptr );
	glCompileShader( fragmentShader );

	// Create and link a shader program, attaching respective shaders to it
	if ( !firstTime )
		glDeleteProgram( defaultProgram );
	
	defaultProgram = glCreateProgram();
	
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
	// Our default triangle
	static const GLfloat triangleVerts[] =
	{
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f,
	};

	// Create a vertex array object
	glGenVertexArrays( 1, &testVertArray );
	glBindVertexArray( testVertArray );

	// Generate a buffer that will contain this vertex data
	glGenBuffers( 1, &testVertBuf );
	glBindBuffer( GL_ARRAY_BUFFER, testVertBuf );
	glBufferData( GL_ARRAY_BUFFER, sizeof( triangleVerts ), triangleVerts, GL_STATIC_DRAW );
}

void Renderer::RenderDefaultModel()
{
	glDisable( GL_CULL_FACE );
	
	// Set up the vertex attribute pointer for location 0
	// layout (location = 0) vec3 vPosition;
	glEnableVertexAttribArray( 4 );
	glBindBuffer( GL_ARRAY_BUFFER, testVertBuf );
	
	glVertexAttribPointer( 4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void*)0 );

	// Render the thing using the default program
	glUseProgram( defaultProgram );
	glDrawArrays( GL_TRIANGLES, 0, 3 );

	glDisableVertexAttribArray( 4 );

	glEnable( GL_CULL_FACE );
}
