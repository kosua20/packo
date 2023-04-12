#include "core/Common.hpp"
#include "core/Settings.hpp"

#include "core/Graph.hpp"
#include "core/nodes/Nodes.hpp"
#include "core/Evaluator.hpp"

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

#include <json/json.hpp>

#include "fonts/font_data_Lato.h"

#include <unordered_map>

#ifdef _WIN32
#ifdef _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"mainCRTStartup\"")
#else
// Avoid command prompt appearing on startup
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif
#endif

unsigned int packedColorFromVec4( const ImVec4& _col )
{
	unsigned char r = ( unsigned int )glm::clamp( std::round( _col.x * 255.f ), 0.f, 255.f );
	unsigned char g = ( unsigned int )glm::clamp( std::round( _col.y * 255.f ), 0.f, 255.f );
	unsigned char b = ( unsigned int )glm::clamp( std::round( _col.z * 255.f ), 0.f, 255.f );
	unsigned char a = ( unsigned int )glm::clamp( std::round( _col.w * 255.f ), 0.f, 255.f );
	return IM_COL32( r, g, b, a );
}

unsigned int packedOpaqueColorFromVec4( const ImVec4& _col )
{
	unsigned char r = ( unsigned int )glm::clamp( std::round( _col.x * 255.f ), 0.f, 255.f );
	unsigned char g = ( unsigned int )glm::clamp( std::round( _col.y * 255.f ), 0.f, 255.f );
	unsigned char b = ( unsigned int )glm::clamp( std::round( _col.z * 255.f ), 0.f, 255.f );
	return IM_COL32( r, g, b, 255 );
}

void autoLayout( const Graph& graph )
{
	// Nodes need to be valid.
	uint nodeCount = 0u;
	GraphNodes nodes(graph);
	ImVec2 maxNodeSize( 100.f, 170.f );
	for( uint node : nodes )
	{
		const ImVec2 nodeSize = ImNodes::GetNodeDimensions( node );
		maxNodeSize.x = ( std::max )( nodeSize.x, maxNodeSize.x );
		maxNodeSize.y = ( std::max )( nodeSize.y, maxNodeSize.y );
		++nodeCount;
	}
	// Add padding 
	maxNodeSize.x *= 1.1f;
	maxNodeSize.y *= 1.1f;

	const uint gridSize = uint( std::ceil( std::sqrt( ( float )nodeCount ) ) );

	GraphNodes nodes2( graph );
	uint id = 0;
	for( uint node : nodes2 )
	{
		ImVec2 pos( ( float )( id % gridSize ), ( float )( id / gridSize ) );
		pos.x *= maxNodeSize.x;
		pos.y *= maxNodeSize.y;
		ImNodes::SetNodeGridSpacePos( node, pos );
		++id;
	}
}

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
	
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2( 8, 8 );
	style.FramePadding = ImVec2( 10, 4 );
	style.CellPadding = ImVec2( 4, 2 );
	style.ItemSpacing = ImVec2( 10, 10 );
	style.ItemInnerSpacing = ImVec2( 4, 4 );
	style.GrabMinSize = 18;
	style.FrameBorderSize = 0;
	style.WindowBorderSize = 0;
	style.FrameRounding = 4;
	style.GrabRounding = 12;
	style.PopupBorderSize = 0;
	style.PopupRounding = 4;
	style.WindowRounding = 12;
	

	ImVec4* colors = style.Colors;
	colors[ ImGuiCol_FrameBg ] = ImVec4( 0.58f, 0.58f, 0.58f, 0.54f );
	colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.26f, 0.84f, 0.98f, 0.40f );
	colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.26f, 0.84f, 0.98f, 0.40f );
	colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.04f, 0.04f, 0.04f, 1.00f );
	colors[ ImGuiCol_CheckMark ] = ImVec4( 0.08f, 0.71f, 0.77f, 1.00f );
	colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.05f, 0.61f, 0.73f, 1.00f );
	colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.03f, 0.69f, 0.82f, 1.00f );
	colors[ ImGuiCol_Button ] = ImVec4( 0.05f, 0.39f, 0.45f, 1.00f );
	colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.05f, 0.61f, 0.73f, 1.00f );
	colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.03f, 0.69f, 0.82f, 1.00f );
	colors[ ImGuiCol_Header ] = ImVec4( 0.05f, 0.39f, 0.45f, 1.00f );
	colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.05f, 0.61f, 0.73f, 1.00f );
	colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.03f, 0.69f, 0.82f, 1.00f );
	colors[ ImGuiCol_SeparatorHovered ] = ImVec4( 0.05f, 0.39f, 0.45f, 1.00f );
	colors[ ImGuiCol_SeparatorActive ] = ImVec4( 0.05f, 0.61f, 0.73f, 1.00f );
	colors[ ImGuiCol_ResizeGrip ] = ImVec4( 0.05f, 0.39f, 0.45f, 1.00f );
	colors[ ImGuiCol_ResizeGripHovered ] = ImVec4( 0.05f, 0.61f, 0.73f, 1.00f );
	colors[ ImGuiCol_ResizeGripActive ] = ImVec4( 0.03f, 0.69f, 0.82f, 1.00f );
	colors[ ImGuiCol_Tab ] = ImVec4( 0.05f, 0.39f, 0.45f, 1.00f );
	colors[ ImGuiCol_TabHovered ] = ImVec4( 0.03f, 0.69f, 0.82f, 1.00f );
	colors[ ImGuiCol_TabActive ] = ImVec4( 0.05f, 0.61f, 0.73f, 1.00f );
	colors[ ImGuiCol_TabUnfocused ] = ImVec4( 0.02f, 0.16f, 0.18f, 1.00f );
	colors[ ImGuiCol_TabUnfocusedActive ] = ImVec4( 0.04f, 0.26f, 0.31f, 1.00f );
	colors[ ImGuiCol_TextSelectedBg ] = ImVec4( 0.04f, 0.26f, 0.31f, 1.00f );
	colors[ ImGuiCol_NavHighlight ] = ImVec4( 0.05f, 0.61f, 0.73f, 1.00f );

	ImNodesStyle& nodesStyle = ImNodes::GetStyle();
	nodesStyle.NodeCornerRounding = 12;
	nodesStyle.PinCircleRadius = 7.f;
	nodesStyle.PinOffset = 0.0f;
	nodesStyle.Flags = ImNodesStyleFlags_GridLines | ImNodesStyleFlags_NodeOutline;
	unsigned int* nodesColors = nodesStyle.Colors;
	nodesColors[ ImNodesCol_NodeBackground ] = IM_COL32( 50, 50, 50, 255 );
	nodesColors[ ImNodesCol_NodeBackgroundHovered ] = IM_COL32( 75, 75, 75, 255 );
	nodesColors[ ImNodesCol_NodeBackgroundSelected ] = IM_COL32( 75, 75, 75, 255 );
	nodesColors[ ImNodesCol_NodeOutline ] = IM_COL32( 100, 100, 100, 255 );
	// title bar colors match ImGui's titlebg colors
	nodesColors[ ImNodesCol_TitleBar ] = packedColorFromVec4( colors[ ImGuiCol_SliderGrab ] );
	nodesColors[ ImNodesCol_TitleBarHovered ] = packedColorFromVec4( colors[ ImGuiCol_SliderGrabActive ] );
	nodesColors[ ImNodesCol_TitleBarSelected ] = packedColorFromVec4( colors[ ImGuiCol_SliderGrabActive ] );
	// link colors match ImGui's slider grab colors
	nodesColors[ ImNodesCol_Link ] = packedColorFromVec4( colors[ ImGuiCol_SliderGrab ] );
	nodesColors[ ImNodesCol_LinkHovered ] = packedColorFromVec4( colors[ ImGuiCol_SliderGrabActive ] );
	nodesColors[ ImNodesCol_LinkSelected ] = packedColorFromVec4( colors[ ImGuiCol_SliderGrabActive ] );
	// pin colors match ImGui's button colors
	nodesColors[ ImNodesCol_Pin ] = packedColorFromVec4( colors[ ImGuiCol_SliderGrab ] );
	nodesColors[ ImNodesCol_PinHovered ] = packedColorFromVec4( colors[ ImGuiCol_SliderGrabActive ] );
	nodesColors[ ImNodesCol_BoxSelector ] = IM_COL32( 133, 133, 133, 30 );
	nodesColors[ ImNodesCol_BoxSelectorOutline ] = IM_COL32( 133, 133, 133, 150 );
	nodesColors[ ImNodesCol_GridBackground ] = IM_COL32( 40, 40, 50, 200 );
	nodesColors[ ImNodesCol_GridLine ] = IM_COL32( 200, 200, 200, 40 );
	nodesColors[ ImNodesCol_GridLinePrimary ] = IM_COL32( 240, 240, 240, 60 );
	// minimap colors
	nodesColors[ ImNodesCol_MiniMapBackground ] = IM_COL32( 25, 25, 25, 150 );
	nodesColors[ ImNodesCol_MiniMapBackgroundHovered ] = IM_COL32( 25, 25, 25, 200 );
	nodesColors[ ImNodesCol_MiniMapOutline ] = IM_COL32( 150, 150, 150, 100 );
	nodesColors[ ImNodesCol_MiniMapOutlineHovered ] = IM_COL32( 150, 150, 150, 200 );
	nodesColors[ ImNodesCol_MiniMapNodeBackground ] = IM_COL32( 200, 200, 200, 100 );
	nodesColors[ ImNodesCol_MiniMapNodeBackgroundHovered ] = IM_COL32( 200, 200, 200, 255 );
	nodesColors[ ImNodesCol_MiniMapNodeBackgroundSelected ] = nodesColors[ ImNodesCol_MiniMapNodeBackgroundHovered ];
	nodesColors[ ImNodesCol_MiniMapNodeOutline ] = IM_COL32( 200, 200, 200, 100 );
	nodesColors[ ImNodesCol_MiniMapLink ] = nodesColors[ ImNodesCol_Link ];
	nodesColors[ ImNodesCol_MiniMapLinkSelected ] = nodesColors[ ImNodesCol_LinkSelected ];
	nodesColors[ ImNodesCol_MiniMapCanvas ] = IM_COL32( 200, 200, 200, 25 );
	nodesColors[ ImNodesCol_MiniMapCanvasOutline ] = IM_COL32( 200, 200, 200, 200 );
	return window;
}

const uint kMaxSlotCount = 8;

uint fromInputSlotToLink(Graph::Slot slot){
	return 3 * (slot.node * kMaxSlotCount + slot.slot);
}

uint fromOutputSlotToLink(Graph::Slot slot){
	return 3 * (slot.node * kMaxSlotCount + slot.slot) + 1;
}

uint fromAttributeToLink( Graph::Slot slot )
{
	return 3 * ( slot.node * kMaxSlotCount + slot.slot ) + 2;
}

Graph::Slot fromLinkToSlot(uint link){
	uint baseLink = link / 3;
	return {baseLink / kMaxSlotCount, (baseLink % kMaxSlotCount)};
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
	const uint errorTitleBar			= IM_COL32( 190, 15, 15, 255 ); 
	const uint errorTitleBarActive		= IM_COL32( 220, 15, 15, 255 ); 
	const uint errorBackground			= IM_COL32( 50, 5, 5, 255 );
	const uint errorBackgroundActive	= IM_COL32( 75, 5, 5, 255 );

	if(!window){
		Log::Error() << "Unable to create window." << std::endl;
		return 1;
	}

	sr_gui_init();

	int winW, winH;

	std::unique_ptr<Graph> graph(new Graph());

	{
		GraphEditor editor(*graph);
		editor.addNode(new InputNode());
		editor.addNode( new OutputNode() ); // 1
		editor.addNode( new OutputNode() ); // 2
		editor.addNode( new AddNode() ); // 3
		editor.addNode( new AddNode() ); // 4

		editor.addLink( 0, 0, 3, 1 );
		editor.addLink( 0, 1, 1, 1 );
		editor.addLink( 0, 1, 2, 0 );

		editor.addLink( 0, 2, 4, 0 );
		editor.addLink( 0, 3, 1, 3 );
		editor.addLink( 0, 3, 2, 2 );

		editor.addLink( 3, 0, 4, 1 );
		editor.addLink( 4, 0, 1, 0 );
		editor.addLink( 4, 0, 3, 0 );

		editor.commit();
	}

	Node* createdNode = nullptr;
	ImVec2 mouseRightClick( 0.f, 0.f );
	ErrorContext errorContext;

	bool needAutoLayout = true;
	

	while(!glfwWindowShouldClose(window)) {

		glfwWaitEventsTimeout(0.1);

		// Screen resolution.
		glfwGetWindowSize(window, &winW, &winH);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		bool wantsExit = false;

		if(ImGui::IsKeyReleased(ImGuiKey_Escape)){
			wantsExit = true;
		}

		// Menus and settings
		{

			if(ImGui::BeginMainMenuBar()){

				if(ImGui::BeginMenu("File")){
					if(ImGui::MenuItem("Validate graph...")){
						validate(*graph, errorContext);
					}
					
					if(ImGui::MenuItem("Execute graph...")){
						char** inputFiles = nullptr;
						int inputCount = 0;
						char* outputDir = nullptr;
						if(sr_gui_ask_load_files("Input directory", "", "", &inputFiles, &inputCount) == SR_GUI_VALIDATED){
							if(sr_gui_ask_directory("Output directory", "", &outputDir) == SR_GUI_VALIDATED){
								if(inputCount > 0 && inputFiles != nullptr && outputDir != nullptr){
									std::vector<std::string> inputs;
									for(uint i = 0u; i < uint(inputCount); ++i){
										inputs.emplace_back(inputFiles[i]);
									}
									std::string output(outputDir);

									evaluate(*graph, errorContext, inputs, output);
								}
							}
						}
					}

					if(ImGui::MenuItem("Debug graph...")){
						evaluate(*graph, errorContext, {"in/0.png", "in/1.png"}, "out/");
					}
					ImGui::Separator();
					if(ImGui::MenuItem("Quit")){
						wantsExit = true;
					}
					ImGui::EndMenu();
				}

				if(ImGui::BeginMenu("Graph")){

					if(ImGui::MenuItem("Open...")){
						int count = 0;
						char** paths = nullptr;
						if(sr_gui_ask_load_files("Load graph", "", "packgraph", &paths, &count) == SR_GUI_VALIDATED){
							if(count != 0){
								const std::string path(paths[0]);
								std::ifstream file(path);
								if(file.is_open()){
									json data = json::parse(file, nullptr, false);
									if(data.is_discarded()){
										errorContext.addError("Unable to parse graph from file at path \"" + path + "\"");
									} else {
										// Issue: numbered inputs/outputs are created before the freelist indices are reset...
										std::unique_ptr<Graph> newGraph(new Graph());
										if(newGraph->deserialize(data)){
											// Remove the current graph.
											graph.reset(nullptr);
											errorContext.clear();
											graph = std::move(newGraph);
											if(data.contains("layout")){
												std::string state = data["layout"];
												ImNodes::LoadCurrentEditorStateFromIniString(state.c_str(), state.size());
											}
										} else {
											errorContext.addError("Unable to deserialize graph from file at path \"" + path + "\"");
										}

									}
									file.close();
								} else {
									errorContext.addError("Unable to load graph from file at path \"" + path + "\"");
								}
							}
						}
					}

					if(ImGui::MenuItem("Save...", nullptr, false, graph != nullptr)){
						char* rawPath = nullptr;
						if(sr_gui_ask_save_file("Save graph", "", "packgraph", &rawPath) == SR_GUI_VALIDATED){
							json data;
							// Graph is guaranteed to exist.
							graph->serialize(data);
							std::string state = ImNodes::SaveCurrentEditorStateToIniString();
							data["layout"] = state;

							std::string path(rawPath);
							std::ofstream file(path);
							if(file.is_open()){
								file << std::setw(4) << data << "\n";
								file.close();
							} else {
								errorContext.addError("Unable to create file at path \"" + path + "\"");
							}
						}
					}

					ImGui::Separator();
					if(ImGui::MenuItem("Reset...")){
						graph.reset(new Graph());
						errorContext.clear();

						GraphEditor editor(*graph);
						editor.addNode(new InputNode());
						editor.addNode( new OutputNode() );
						for(uint i = 0; i < 4; ++i)
							editor.addLink( 0, i, 1, i );
						editor.commit();

						needAutoLayout = true;
					}
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

		// TODO: Nodes
		// * float AND RGBA versions?
		// * comparisons and boolean selector? (still floats)
		// * "full image" nodes: resize, flip, rotate, blur?
		// * procedural nodes
		// * ...

		// TODO: display preview of inputs/outputs if inputs have been selected?

		const unsigned int winFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar;

		ImGui::SetNextWindowPos(ImVec2(0.0f, menuBarHeight));
		ImGui::SetNextWindowSize(ImVec2(float(winW), float(winH) - menuBarHeight));

		if(ImGui::Begin("PackoMainWindow", nullptr, winFlags)){

			// Graph viewer.
			{
				ImNodes::BeginNodeEditor();
				// Allowing unplugging of links conflicts with multiple outputs
				ImNodes::PushAttributeFlag( 0 /*ImNodesAttributeFlags_EnableLinkDetachWithDragClick*/ );

				// First, immediately register the position for a newly created node if there is one.
				{
					int createdNodeIndex = graph->findNode( createdNode );
					if( createdNodeIndex >= 0 )
						ImNodes::SetNodeScreenSpacePos( createdNodeIndex, mouseRightClick );
					createdNode = nullptr;
				}


				GraphNodes nodes(*graph);

				for(const uint nodeId : nodes){
					 Node* node = graph->node(nodeId);
					const bool nodeHasIssue = errorContext.contains( node );
					if( nodeHasIssue )
					{
						ImNodes::PushColorStyle( ImNodesCol_TitleBar, errorTitleBar );
						ImNodes::PushColorStyle( ImNodesCol_TitleBarHovered, errorTitleBarActive );
						ImNodes::PushColorStyle( ImNodesCol_TitleBarSelected, errorTitleBarActive );
						ImNodes::PushColorStyle( ImNodesCol_NodeBackground, errorBackground );
						ImNodes::PushColorStyle( ImNodesCol_NodeBackgroundHovered, errorBackgroundActive );
						ImNodes::PushColorStyle( ImNodesCol_NodeBackgroundSelected, errorBackgroundActive );
					}

					ImNodes::BeginNode(nodeId);

					ImNodes::BeginNodeTitleBar();
					ImGui::TextUnformatted(node->name().c_str());
					ImNodes::EndNodeTitleBar();

					const std::vector<std::string>& inputs = node->inputs();
					const std::vector<std::string>& outputs = node->outputs();
					std::vector<Node::Attribute>& attributes = node->attributes();
					const uint attributeCount = attributes.size();

					const uint attribWidth = 130u;

					const uint nodeSize = attributeCount != 0u ? attribWidth : 50u;

					for(uint attId = 0; attId < attributeCount; ++attId ){

						ImGui::PushItemWidth(attribWidth);
						Node::Attribute& attribute = attributes[attId];
						switch( attribute.type )
						{
							case Node::Attribute::Type::FLOAT:
								ImGui::InputFloat( attribute.name.c_str(), &attribute.flt, 0.0f, 0.0f, "%.6f" );
								break;
							case Node::Attribute::Type::COLOR:
								ImGui::ColorEdit4( attribute.name.c_str(), &attribute.clr[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR );
								break;
							case Node::Attribute::Type::STRING:
								ImGui::InputText( attribute.name.c_str(), &attribute.str[0], MAX_STR_LENGTH );
								break;
							default:
								assert( false );
								break;
						}
						ImGui::PopItemWidth();
					}

					const uint inputCount = inputs.size();
					const uint outputCount = outputs.size();
					const uint maxCount = std::max(inputCount, outputCount);

					for(uint attId = 0; attId < maxCount; ++attId ){
						const bool hasInput = attId < inputCount;
						const bool hasOutput = attId < outputCount;
						
						if( hasInput ){
							const std::string& name = inputs[attId];
							ImNodes::BeginInputAttribute(fromInputSlotToLink({nodeId, attId}));
							//ImGui::PushItemWidth( slotWidth );
							ImGui::TextUnformatted(name.c_str());
							//ImGui::PopItemWidth();
							ImNodes::EndInputAttribute();
						}
						if(hasOutput){
							bool indented = false;
							const std::string& name = outputs[ attId ];

							const uint offset = nodeSize -  ImGui::CalcTextSize( name.c_str()).x;
							if(hasInput){
								ImGui::SameLine(offset);
							} else {
								ImGui::Indent(offset);
								indented = true;
							}
							ImNodes::BeginOutputAttribute(fromOutputSlotToLink({nodeId, attId}));
							//ImGui::PushItemWidth( slotWidth );
							ImGui::TextUnformatted(name.c_str());
							//ImGui::PopItemWidth();
							ImNodes::EndOutputAttribute();
							if(indented){
								ImGui::Unindent(offset);
							}
						}
					}

					ImNodes::EndNode();
					if( nodeHasIssue ) {
						for(uint i = 0; i < 6; ++i )
						ImNodes::PopColorStyle();
					}
				}

				uint linkCount = graph->getLinkCount();
				for(uint linkId = 0u; linkId < linkCount; ++linkId ){
					const Graph::Link& link = graph->link( linkId );
					ImNodes::Link(linkId, fromOutputSlotToLink(link.from), fromInputSlotToLink(link.to));
				}

				ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
				ImNodes::PopAttributeFlag();
				ImNodes::EndNodeEditor();

				if( needAutoLayout )
				{
					autoLayout( *graph );
					needAutoLayout = false;
				}
			}

			// Graph edition.
			{
				GraphEditor editor(*graph);

				int startLink, endLink;
				if(ImNodes::IsLinkCreated(&startLink, &endLink)){
					Graph::Slot from = fromLinkToSlot(startLink);
					Graph::Slot to = fromLinkToSlot(endLink);
					editor.addLink(from.node, from.slot, to.node, to.slot);
				}

				int linkId;
				if(ImNodes::IsLinkDestroyed(&linkId)){
					editor.removeLink(linkId);
				}

				if(ImGui::IsKeyReleased(ImGuiKey_Delete) ||
				   (ImGui::IsKeyReleased(ImGuiKey_Backspace) && (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)))){
					const uint nodesCount = ImNodes::NumSelectedNodes();
					if( nodesCount > 0u )
					{
						std::vector<int> nodeIds( nodesCount );
						ImNodes::GetSelectedNodes( nodeIds.data() );
						for( const int nodeId : nodeIds )
						{
							editor.removeNode( ( uint )nodeId );
						}
					}
					const uint linkCount = ImNodes::NumSelectedLinks();
					if( linkCount > 0 )
					{
						std::vector<int> linkIds( linkCount );
						ImNodes::GetSelectedLinks( linkIds.data() );
						for( const int linkId : linkIds )
						{
							editor.removeLink( ( uint )linkId );
						}
					}
				}

				if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
					ImGui::OpenPopup( "Create node" );
					// Save position for placing the new node on screen.
					mouseRightClick = ImGui::GetMousePos();
				}

				if( ImGui::BeginPopup("Create node")){
					NodeClass typeToCreate = NodeClass::COUNT_EXPOSED;

					for(uint i = 0; i < NodeClass::COUNT_EXPOSED; ++i){
						NodeClass type = NodeClass(i);
						const std::string& label = getNodeName(type);
						if( ImGui::Selectable(label.c_str())){
							typeToCreate = type;
						}
					}
					if(typeToCreate < NodeClass::COUNT_EXPOSED){
						createdNode = createNode(typeToCreate);
						editor.addNode( createdNode );
					}
					ImGui::EndPopup();
				}
				
				editor.commit();
			}

		}
		const float mainWindowHeight = ImGui::GetWindowHeight();
		const float mainWindowWidth = ImGui::GetWindowWidth();
		ImGui::End();

		if(errorContext.hasErrors()){

			ImGui::SetNextWindowPos(ImVec2(0, mainWindowHeight + menuBarHeight), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
			if(ImGui::Begin("Error messages", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)){
				const uint errorCount = errorContext.errorCount();

				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
				ImGui::Text( "Graph validation: %u errors", errorCount );
				ImGui::PopStyleColor();

				if(ImGui::BeginTable("##Errors", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg, ImVec2(400,0))){
					ImGui::TableSetupColumn("Node");
					ImGui::TableSetupColumn("Slot");
					ImGui::TableSetupColumn("Message");
					ImGui::TableHeadersRow();
					for( uint i = 0; i < errorCount; ++i ){

						ImGui::PushID(i);
						const char* msg;
						const Node* node;
						int slot;
						errorContext.getError( i, msg, node, slot );

						ImGui::TableNextColumn();
						if(node){
							if( ImGui::Selectable( node->name().c_str(), false, ImGuiSelectableFlags_SpanAllColumns ) )
							{
								uint nodeId = graph->findNode( node );
								ImNodes::EditorContextMoveToNode( nodeId );
								ImVec2 pan = ImNodes::EditorContextGetPanning();
								ImVec2 size = ImNodes::GetNodeDimensions(nodeId);
								pan.x += 0.5f * (mainWindowWidth - size.x);
								pan.y += 0.5f * (mainWindowHeight - size.y);
								ImNodes::EditorContextResetPanning( pan );
							}
						}

						ImGui::TableNextColumn();
						if(slot >= 0){
							if(node && ((uint)slot < node->inputs().size())){
								ImGui::TextUnformatted(node->inputs()[slot].c_str());
							} else {
								ImGui::Text("%d", slot + 1);
							}
						}
						ImGui::TableNextColumn();
						ImGui::TextUnformatted( msg );
						ImGui::PopID();

					}

					ImGui::EndTable();
				}
			}
			ImGui::End();
		}

		// We *might* want to exit, ask the user for confirmation.
		if(wantsExit){
			ImGui::OpenPopup("Exit");
		}

		// Exit confirmation popup
		if(ImGui::BeginPopupModal("Exit", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)){
			ImGui::Text("Are you sure you want to quit?");
			const ImVec2 buttonSize(150.0f, 0.0f);
			if(ImGui::Button("No", buttonSize)){
				wantsExit = false;
				// This will close the popup.
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine(buttonSize.x+20.f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
			if(ImGui::Button("Yes", buttonSize)){
				// Now we definitely want to exit.
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}

		// Render the interface.
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		glfwSwapBuffers(window);
	}

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
