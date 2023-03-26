#include "core/Common.hpp"

#include "core/system/Config.hpp"
#include "core/system/System.hpp"
#include "core/system/TextUtilities.hpp"

#include <gl3w/gl3w.h>
#include <GLFW/glfw3.h>
#include <sr_gui/sr_gui.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "fonts/font_data_Lato.h"

#ifdef _WIN32
// Avoid command prompt appearing on startup
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

GLFWwindow* createWindow(int w, int h, UIStyle& uiStyle) {

	// Initialize glfw, which will create and setup an OpenGL context.
	if(!glfwInit()) {
		return NULL;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	const unsigned int ww = std::max( mode->width/2, w);
	const unsigned int hh = std::max( mode->height/2, h);

	GLFWwindow* window = glfwCreateWindow(ww, hh, "Packo", NULL, NULL);

	if(!window) {
		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(window);

	if(gl3wInit()) {
		return NULL;
	}
	if(!gl3wIsSupported(3, 2)) {
		return NULL;
	}

	glfwSwapInterval(1);

	ImGui::CreateContext();
	
	ImFontConfig fontLato = ImFontConfig();
	fontLato.FontData = (void*)(fontDataLato);
	fontLato.FontDataSize = size_fontDataLato;
	fontLato.SizePixels = 18.0f;
	// Font data is static
	fontLato.FontDataOwnedByAtlas = false;

	ImGuiIO & io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.Fonts->AddFont(&fontLato);

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	return window;
}


int main(int argc, char** argv){

	PackoConfig config(std::vector<std::string>(argv, argv+argc));
	if(config.version){
		Log::Info() << versionMessage << std::endl;
		return 0;
	} else if(config.license){
		Log::Info() << licenseMessage << std::endl;
		return 0;
	} else if(config.bonus){
		Log::Info() << bonusMessage << std::endl;
		return 0;
	} else if(config.showHelp(false)){
		return 0;
	}

	GLFWwindow* window = createWindow(830, 620, style);

	if(!window){
		Log::Error() << "Unable to create window." << std::endl;
		return 1;
	}

	sr_gui_init();

	int winW, winH;

	while(!glfwWindowShouldClose(window)) {

		glfwWaitEventsTimeout(0.1);

		// Screen resolution.
		glfwGetWindowSize(window, &winW, &winH);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Menus and settings
		{

			if(ImGui::BeginMainMenuBar()){

				if(ImGui::BeginMenu("File")){

					if(ImGui::BeginMenu("Open...")){
						ImGui::EndMenu();
					}


					ImGui::Separator();
					if(ImGui::MenuItem("Quit")){
						glfwSetWindowShouldClose(window, GLFW_TRUE);
					}
					ImGui::EndMenu();
				}

				if(ImGui::BeginMenu("View")){
					//ImGui::MenuItem("Nodes", nullptr, &showLibrary, true);
					ImGui::EndMenu();
				}

				if(ImGui::BeginMenu("About")){
					ImGui::Text( "%s", versionMessage.c_str() );
					ImGui::Text( "© Simon Rodriguez 2023" );
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}

		}



		const float heightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		
		ImGui::SetNextWindowPos(ImVec2(0.0f, menuBarHeight));
		ImGui::SetNextWindowSize(ImVec2(float(winW), float(winH) - menuBarHeight));



		if(ImGui::Begin("PackoMainWindow", nullptr, winFlags)){


		}
		ImGui::End();


		// Render the interface.
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwSwapBuffers(window);
	}

	// Save internal state.

	// Cleanup.
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	sr_gui_cleanup();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
