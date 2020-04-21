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

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplOpenGL2_Init();

	io.DisplaySize.x = ScreenWidth;
	io.DisplaySize.y = ScreenHeight;
}

void CClientImgui::DrawImgui()
{
	ImGui_ImplSDL2_NewFrame( mainWindow );
	ImGui_ImplOpenGL2_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin( "Test" );
	ImGui::Text( "uwu" );
	ImGui::End();

	//glViewport( 0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y );

	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData() );
	SDL_GL_SwapWindow( mainWindow );
}