#include "HUD/hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"
#include "SDL2/SDL.h"
#include <gl/GL.h>

#include "imgui.h"
#include "examples/imgui_impl_opengl2.h"
#include "examples/imgui_impl_sdl.h"

SDL_Window* mainWindow;

void CClientImgui::InitExtension()
{
	mainWindow = SDL_GetWindowFromID( 1 );

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	
	// Setup Platform/Renderer bindings
	ImGui_ImplOpenGL2_Init();
	io.DisplaySize.x = ScreenWidth;
	io.DisplaySize.y = ScreenHeight;
}

void CClientImgui::DrawImgui()
{
	static float angle = 0;
	angle += M_PI * 2.0 / 180;

	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame( mainWindow );
	ImGui::NewFrame();

	ImGui::SetNextWindowSize( ImVec2( 200 + cos(angle)*50, 200 + sin(angle)*50 ) );

	ImGui::Begin( "Test" );

	ImGui::Text( "uwu" );
	ImGui::Text( "Nyaaaa!!!" );
	ImGui::Button( "OwO" );

	ImGui::End();

	//glViewport( 0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y );

	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData() );
}