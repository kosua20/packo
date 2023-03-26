#include "core/Common.hpp"
#include "core/Settings.hpp"

#include "core/system/Config.hpp"
#include "core/system/System.hpp"
#include "core/system/TextUtilities.hpp"
#include "core/Strings.hpp"

#include <gl3w/gl3w.h>
#include <GLFW/glfw3.h>
#include <sr_gui/sr_gui.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imnodes/imnodes.h>

#include "fonts/font_data_Lato.h"

#include <unordered_map>

#ifdef _WIN32
// Avoid command prompt appearing on startup
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

GLFWwindow* createWindow(int w, int h) {

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
	ImNodes::CreateContext();
	
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


	ImNodes::GetIO().EmulateThreeButtonMouse.Modifier = &ImGui::GetIO().KeyAlt;
	ImNodes::GetIO().LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;
	
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

	GLFWwindow* window = createWindow(830, 620);

	if(!window){
		Log::Error() << "Unable to create window." << std::endl;
		return 1;
	}

	sr_gui_init();

	int winW, winH;
	float val;
	std::unordered_map<int, std::pair<int, int>> links;
	int linkIndex = 0;

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
		const float menuBarHeight = ImGui::GetItemRectSize().y;

		const unsigned int winFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar;

		ImGui::SetNextWindowPos(ImVec2(0.0f, menuBarHeight));
		ImGui::SetNextWindowSize(ImVec2(float(winW), float(winH) - menuBarHeight));


		if(ImGui::Begin("PackoMainWindow", nullptr, winFlags)){


			ImNodes::BeginNodeEditor();

			ImNodes::BeginNode(1);
			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("output node");
			ImNodes::EndNodeTitleBar();
			ImNodes::BeginOutputAttribute(10);
			ImGui::Text("output pin");
			ImNodes::EndOutputAttribute();
			ImNodes::EndNode();

			ImNodes::BeginNode(2);
			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("mixed node");
			ImNodes::EndNodeTitleBar();
			ImNodes::BeginStaticAttribute(21);
			ImGui::PushItemWidth(80);
			ImGui::SliderFloat("test", &val, 0.f, 1.f);
			ImGui::PopItemWidth();
			ImNodes::EndStaticAttribute();
			ImNodes::BeginInputAttribute(20);
			ImGui::Text("intput pin");
			ImNodes::EndInputAttribute();
			ImNodes::BeginOutputAttribute(22);
			ImGui::Text("output pin 1");
			ImNodes::EndOutputAttribute();
			ImNodes::BeginOutputAttribute(23);
			ImGui::Text("output pin 2");
			ImNodes::EndOutputAttribute();
			ImNodes::EndNode();

			ImNodes::BeginNode(3);
			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("output node");
			ImNodes::EndNodeTitleBar();
			ImNodes::BeginInputAttribute(30);
			ImGui::Text("input pin 1");
			ImNodes::EndInputAttribute();
			ImNodes::BeginInputAttribute(31);
			ImGui::Text("input pin 2");
			ImNodes::EndInputAttribute();
			ImNodes::EndNode();

			for(const auto& link : links){
				ImNodes::Link(link.first, link.second.first, link.second.second);

			}
			ImNodes::MiniMap();
			ImNodes::EndNodeEditor();

			int startLink, endLink;
			if(ImNodes::IsLinkCreated(&startLink, &endLink)){
				links[linkIndex++] = {startLink, endLink};
			}

			int linkId;
			if(ImNodes::IsLinkDestroyed(&linkId)){
				links.erase(linkId);
			}
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
	ImNodes::DestroyContext();
	ImGui::DestroyContext();

	sr_gui_cleanup();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
