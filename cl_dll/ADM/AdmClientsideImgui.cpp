#include "HUD/hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "PlayerMove/pm_shared.h"
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

void CClientImgui::TestDrawImgui()
{
	static float angle = 0;
	angle += M_PI * 2.0 / 180;

	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame( mainWindow );
	ImGui::NewFrame();

	ImGui::SetNextWindowSize( ImVec2( 200 + cos(angle)*50, 200 + sin(angle)*50 ) );
	ImGui::SetNextWindowBgAlpha( cos(angle*4.0) );

	ImGui::Begin( "Test" );

	ImGui::Text( "uwu" );
	ImGui::Text( "Nyaaaa!!!" );
	ImGui::Button( "OwO" );

	ImGui::End();	
}

void CClientImgui::DisplayImgui()
{
	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData() );
}

void CClientImgui::CloseImgui()
{
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}