#include "core/Common.hpp"
#include "core/Random.hpp"

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
#include "app/ImGuiHelpers.hpp"

#include <json/json.hpp>

#include <unordered_map>

#ifdef _WIN32
#ifdef _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"mainCRTStartup\"")
#else
// Avoid command prompt appearing on startup
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif
#endif

/// Window & GPU handling

GLFWwindow* createWindow(int w, int h, ImFont*& defaultFont, ImFont*& smallFont) {

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

	applyStyleImGuiAndImNodes(defaultFont, smallFont);

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	sr_gui_init();

	return window;
}

void purgeTextures( std::unordered_map<const Node*, GLuint>& textures ){
	for(const auto& texture : textures){
		GLuint tex = texture.second;
		glDeleteTextures(1, &tex);
	}
	textures.clear();
}

/// Graph layout

void autoLayout(const Graph& graph){
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

Graph* createDefaultGraph(){
	Graph* graph = new Graph();

	GraphEditor editor(*graph);
	editor.addNode(new InputNode());
	editor.addNode(new OutputNode());

	for(uint i = 0; i < 4; ++i)
		editor.addLink( 0, i, 1, i );

	editor.commit();

	return graph;
}

bool loadGraph(std::unique_ptr<Graph>& graph, ErrorContext& errorContext){
	int count = 0;
	char** paths = nullptr;
	if(sr_gui_ask_load_files("Load graph", "", "packgraph", &paths, &count) != SR_GUI_VALIDATED){
		return false;
	}
	if(count == 0 || !paths || !paths[0]){
		if(paths){
			free(paths);
		}
		return false;
	}
	const std::string path(paths[0]);
	// Immediately clean up the paths.
	for(int i = 0; i < count; ++i){
		free(paths[i]);
	}
	free(paths);

	std::ifstream file(path);
	if(!file.is_open()){
		errorContext.addError("Unable to load graph from file at path \"" + path + "\"");
		return false;
	}

	json data = json::parse(file, nullptr, false);
	// Done with the file.
	file.close();
	// Invalid JSON.
	if(data.is_discarded()){
		errorContext.addError("Unable to parse graph from file at path \"" + path + "\"");
		return false;
	}

	// Issue: numbered inputs/outputs are created before the freelist indices are reset...
	// So we have to destroy the current graph first.
	// In case of rollback, use a serialized copy of the old graph and hope for the best.
	json oldGraph;
	graph->serialize( oldGraph );
	graph.reset(new Graph());
	errorContext.clear();

	if(!graph->deserialize(data)){
		errorContext.addError("Unable to deserialize graph from file at path \"" + path + "\"");
		// Restore the previous graph.
		graph.reset( new Graph() );
		graph->deserialize( oldGraph );
		return true;
	}
	if(data.contains("layout")){
		std::string state = data["layout"];
		ImNodes::LoadCurrentEditorStateFromIniString(state.c_str(), state.size());
	}
	return true;
}

void saveGraph(std::unique_ptr<Graph>& graph, ErrorContext& errorContext){
	char* rawPath = nullptr;
	if(sr_gui_ask_save_file("Save graph", "", "packgraph", &rawPath) != SR_GUI_VALIDATED){
		if(rawPath)
			free(rawPath);
		return;
	}

	json data;
	// Graph is guaranteed to exist.
	graph->serialize(data);
	std::string state = ImNodes::SaveCurrentEditorStateToIniString();
	data["layout"] = state;

	std::string path(rawPath);
	free(rawPath);

	std::ofstream file(path);
	if(!file.is_open()){
		errorContext.addError("Unable to create file at path \"" + path + "\"");
		return;
	}

	file << std::setw(4) << data << "\n";
	file.close();
}

/// Input/output files

struct InputFile {
	fs::path path;
	bool active;
};

bool refreshFiles(const fs::path& dir, std::vector<InputFile>& paths){
	static const std::vector<std::string> validExts = {"png", "bmp", "tga", "jpeg"};
	if(!fs::exists(dir)){
		paths.clear();
		return false;
	}
	std::vector<InputFile> newPaths;
	// Don't recurse
	for (const fs::directory_entry& file : fs::directory_iterator(dir)) {
		const fs::path& path = file.path();
		std::string ext = path.extension().string();
		ext = TextUtilities::trim( ext, ".");
		if(std::find(validExts.begin(), validExts.end(), ext) != validExts.end()){
			newPaths.emplace_back();
			newPaths.back().path = path;
		}
	}
	std::sort(newPaths.begin(), newPaths.end(), [](const InputFile& a, const InputFile& b){
		return a.path < b.path;
	});

	// Transfer existing active status.
	const uint newCount = newPaths.size();
	const uint oldCount = paths.size();
	uint reusedCount = 0u;
	uint currentOld = 0u;
	for(uint currentNew = 0u; currentNew < newCount; ++currentNew){
		const fs::path& currentPath = newPaths[currentNew].path;
		while((currentOld < oldCount) && (paths[currentOld].path != currentPath)){
			++currentOld;
		}
		if(currentOld < oldCount){
			++reusedCount;
			newPaths[currentNew].active = paths[currentOld].active;
		} else {
			newPaths[currentNew].active = true;
		}
		// No need to reset currentOld, paths are sorted.
	}
	// Done, swap
	std::swap(paths, newPaths);
	return (reusedCount != newCount) || (newCount != oldCount);
}

std::vector<fs::path> filterInputFiles(std::vector<InputFile>& files){
	std::vector<fs::path> inputPaths;
	for(InputFile& file : files){
		if(file.active){
			inputPaths.push_back( file.path );
		}
	}
	return inputPaths;
}

enum DirectoryType : int {
	INPUT_IMAGES = 1 << 0,
	OUTPUT_IMAGES = 1 << 1,
	BOTH = INPUT_IMAGES | OUTPUT_IMAGES
};

void askForDirectories(DirectoryType types, fs::path& inputDirectory, fs::path& outputDirectory){

	if(types & INPUT_IMAGES ){
		char* rawPath = nullptr;
		if(sr_gui_ask_directory("Input directory", "", &rawPath) == SR_GUI_VALIDATED){
			if(rawPath){
				inputDirectory = rawPath;
			}
		}
		if(rawPath){
			free(rawPath);
		}
	}
	if(types & OUTPUT_IMAGES ){
		char* rawPath = nullptr;
		if(sr_gui_ask_directory("Output directory", "", &rawPath) == SR_GUI_VALIDATED){
			if(rawPath){
				outputDirectory = rawPath;
			}
		}
		if(rawPath){
			free(rawPath);
		}
	}
}

/// GUI Panels

struct Styling {
	ImFont* defaultFont;
	ImFont* smallFont;
	float kSplitBarWidth;
	uint kMaxRefreshDelayInFrames;
	ImNodesPinShape kPinsShape;
	float kPreviewDisplayWidth;
	float kSlotLabelWidth;
	float kNodeInternalWidth;
	float kNodeTotalWidth;
	unsigned int kWinFlags;
	float kSafetyMargin;
};

const Node* showErrorPanel(const ErrorContext& errorContext, float winWidth, float winHeight){
	const int kWinFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

	const Node* nodeToFocus = nullptr;

	ImGui::SetNextWindowPos(ImVec2(winWidth, winHeight), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
	if(ImGui::Begin("Error messages", nullptr, kWinFlags)){

		const uint errorCount = errorContext.errorCount();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::Text( "Graph validation: %u errors", errorCount );
		ImGui::PopStyleColor();

		ImGui::BeginTable("##Errors", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg, ImVec2(400,0));
		ImGui::TableSetupColumn("Node");
		ImGui::TableSetupColumn("Slot");
		ImGui::TableSetupColumn("Message");
		ImGui::TableHeadersRow();
		for( uint i = 0; i < errorCount; ++i ){

			ImGui::PushID(i);
			const char* msg;
			const Node* node;
			int slot;
			errorContext.getError(i, msg, node, slot);

			ImGui::TableNextColumn();
			if(node){
				if(ImGui::Selectable( node->name().c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)){
					nodeToFocus = node;
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
	ImGui::End();
	return nodeToFocus;
}

bool drawCommentNode(Node* node, uint nodeId, const Styling& style){
	ImNodes::BeginNode( nodeId );
	Node::Attribute& att = node->attributes()[ 0 ];
	// Count line returns.
	uint lineCount = 1;
	for(char c : att.str){
		if(c == '\n'){
			++lineCount;
		}
	}

	const ImVec2 fieldSize(style.kNodeInternalWidth, (float(lineCount) + 1.5f) * ImGui::GetTextLineHeight());
	TextCallbackInfo commentInfo{att.str.c_str(), fieldSize.x - style.kSafetyMargin };
	ImGui::InputTextMultiline( "##Comment", &(att.str), fieldSize, ImGuiInputTextFlags_CallbackEdit, &commentTextCallback, (void*)&commentInfo);
	const bool edited = ImGui::IsItemActive();

	ImNodes::EndNode();
	return edited;
}

bool drawNode(Node* node, uint nodeId, const Styling& style, const std::unordered_map<const Node*, GLuint>& textures, int& nodeToPurgeLinks){
	bool editedGraph = false;
	ImNodes::BeginNode(nodeId);

	ImNodes::BeginNodeTitleBar();
	{
		if(node->channeled()){
			const char* labels[] = {"0", "1", "2", "3", "4"};
			const uint channelCount = node->channelCount();
			if(ImGui::SmallButton( labels[channelCount])){
				node->setChannelCount(channelCount % 4 + 1);
				editedGraph = true;
				// We'll have to remove extraneous links.
				nodeToPurgeLinks = int(nodeId);
			}
			ImGui::SameLine(style.kSlotLabelWidth);
			ImGui::TextUnformatted( node->name().c_str() );
		} else {
			ImGui::Indent(style.kSlotLabelWidth);
			ImGui::TextUnformatted( node->name().c_str() );
			ImGui::Unindent();
		}
		ImGui::SameLine(style.kPreviewDisplayWidth + style.kSlotLabelWidth * 1.1f);
		ImGui::Bullet();
		if( ImGui::IsItemHovered()){
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10,10));
			ImGui::SetTooltip( "%s", node->description().c_str());
			ImGui::PopStyleVar();
		}
	}
	ImNodes::EndNodeTitleBar();

	// Draw all inputs.
	ImGui::BeginGroup();
	{
		const bool multiChannel = (node->channelCount() > 1) && (node->channeledInputs());
		uint slotId = 0u;
		for( const std::string& name : node->inputs() ){
			ImNodes::BeginInputAttribute( fromInputSlotToLink( { nodeId, slotId } ), style.kPinsShape);
			TextIndex( name, multiChannel, style.smallFont );
			ImNodes::EndInputAttribute();
			++slotId;
		}
		// Force group size.
		ImGui::Dummy(ImVec2(style.kSlotLabelWidth, 0));
	}
	ImGui::EndGroup();

	ImGui::SameLine(0,0);
	ImGui::BeginGroup();
	// Draw texture preview.
	{
		auto texKey = textures.find( node );
		if(texKey != textures.end()){
			GLuint tex = texKey->second;
			ImGui::Image( ( ImTextureID )(uintptr_t)tex, ImVec2( style.kPreviewDisplayWidth, style.kPreviewDisplayWidth ) );
		} else{
			// Ensure node width is always the same.
			ImGui::Dummy( ImVec2( style.kPreviewDisplayWidth, 0.f ) );
		}
	}
	// Draw attributes
	{
		// Compute size of all attributes (for nice alignment)
		float minAttributeSize = style.kPreviewDisplayWidth;
		for( const Node::Attribute& attribute : node->attributes() )
		{
			// Subtract label size if not hidden.
			float attLabelSize = ImGui::CalcTextSize( attribute.name.c_str(), nullptr, true ).x;
			if( attLabelSize != 0.f ){
				attLabelSize += ImGui::GetStyle().ItemInnerSpacing.x;
			}
			minAttributeSize = std::min( minAttributeSize, style.kPreviewDisplayWidth - attLabelSize );
		}
		minAttributeSize = std::max( 0.f, minAttributeSize );

		for( Node::Attribute& attribute : node->attributes() ){

			ImGui::PushItemWidth( minAttributeSize );
			switch( attribute.type )
			{
				case Node::Attribute::Type::FLOAT:
					editedGraph |= ImGui::DragFloat( attribute.name.c_str(), &attribute.flt, 0.05f, 0.0f, 0.0f, "%.6f" );
					break;
				case Node::Attribute::Type::COLOR:
					editedGraph |= ImGui::ColorPicker4( attribute.name.c_str(), &attribute.clr[ 0 ], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayHex | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_NoSmallPreview );
					break;
				case Node::Attribute::Type::STRING:
					editedGraph |= ImGui::InputText( attribute.name.c_str(), &(attribute.str) );
					break;
				case Node::Attribute::Type::COMBO:
					editedGraph |= ImGui::Combo( attribute.name.c_str(), &attribute.cmb, &getAttributeComboItemCallback, &attribute, attribute.values.size() );
					break;
				case Node::Attribute::Type::BOOL:
					editedGraph |= ImGui::Checkbox( attribute.name.c_str(), &attribute.bln );
					break;
				case Node::Attribute::Type::VEC3:
					editedGraph |= ImGui::DragFloat3( attribute.name.c_str(), &attribute.clr[0], 0.05f, 0.0f, 0.0f, "%.6f" );
					break;
				default:
					assert( false );
					break;
			}
			ImGui::PopItemWidth();
		}
	}
	ImGui::EndGroup();

	// Draw all outputs
	ImGui::SameLine(0,0);
	ImGui::BeginGroup();
	{
		const bool multiChannel = (node->channelCount() > 1) && (node->channeledOutputs());
		uint slotId = 0u;
		for( const std::string& name : node->outputs() ){

			const float labelSize = TextIndexSize( name, multiChannel, style.smallFont->FontSize / style.defaultFont->FontSize );
			const float offset = std::max( 0.f, style.kSlotLabelWidth - labelSize );

			ImNodes::BeginOutputAttribute( fromOutputSlotToLink( { nodeId, slotId } ), style.kPinsShape);
			ImGui::Indent(offset);
			TextIndex( name, multiChannel, style.smallFont );
			ImGui::Unindent();
			ImNodes::EndOutputAttribute();
			++slotId;
		}
		// Force group size.
		ImGui::Dummy(ImVec2(style.kSlotLabelWidth, 0));
	}
	ImGui::EndGroup();

	ImNodes::EndNode();
	return editedGraph;
}

void showCreationPopup(std::unordered_map<NodeClass, uint>& nodesToCreate, std::string& searchStr, int& selectedNodeType, std::vector<NodeClass>& visibleNodeTypes, bool focusTextField) {

	int visibleTypesCount = int(visibleNodeTypes.size());
	const int columnWidth = 160;
	const int columnItemsHeight = 14;
	const int columnCount = (visibleTypesCount + columnItemsHeight - 1 ) / columnItemsHeight;

	if(ImGui::IsKeyPressed(ImGuiKey_UpArrow, true)){
		--selectedNodeType;
		if(selectedNodeType < 0){
			selectedNodeType = visibleTypesCount - 1;
		}
	}
	if(ImGui::IsKeyPressed(ImGuiKey_DownArrow, true)){
		++selectedNodeType;
		if(selectedNodeType >= visibleTypesCount){
			selectedNodeType = 0;
		}
	}
	if(ImGui::IsKeyPressed(ImGuiKey_RightArrow, true)){
		selectedNodeType += columnItemsHeight;
		if(selectedNodeType >= visibleTypesCount){
			selectedNodeType = selectedNodeType % columnItemsHeight;
		}
	}
	if(ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true)){
		selectedNodeType -= columnItemsHeight;
		if(selectedNodeType < 0){
			// Is there a simpler expression?
			selectedNodeType = (selectedNodeType + columnItemsHeight) % columnItemsHeight + (columnCount - 1) * columnItemsHeight;
			// Only the last column can be not full.
			if(selectedNodeType >= visibleTypesCount){
				selectedNodeType -= columnItemsHeight;
			}
		}
	}

	ImGui::PushItemWidth(columnWidth * columnCount);

	if(focusTextField){
		ImGui::SetKeyboardFocusHere();
	}

	if(ImGui::InputText("##SearchField", &searchStr, ImGuiInputTextFlags_AutoSelectAll)){
		// Refresh selection.
		const std::string searchStrLow = TextUtilities::lowercase(searchStr);
		visibleNodeTypes.clear();
		visibleNodeTypes.reserve(NodeClass::COUNT_EXPOSED);
		// Filter visible types.
		for(uint i = 0; i < NodeClass::COUNT_EXPOSED; ++i){
			const NodeClass type = getOrderedType(i);
			const std::string labelLow = TextUtilities::lowercase(getNodeName(type));
			if(labelLow.find(searchStrLow) == std::string::npos)
				continue;
			visibleNodeTypes.push_back(type);
		}
		selectedNodeType = 0;
		visibleTypesCount = visibleNodeTypes.size();
	}
	ImGui::PopItemWidth();

	for(int i = 0; i < columnItemsHeight; ++i){
		// Split in multiple columns.
		for(int c = 0; c < columnCount; ++c){
			const int index = c * columnItemsHeight + i;
			if( index >= visibleTypesCount )
				break;

			const NodeClass type = visibleNodeTypes[index];
			const std::string& label = getNodeName(type);
			const ImVec2 selectSize = ImVec2(columnWidth - 2 * ImGui::GetStyle().FramePadding.x, 0);
			if(ImGui::Selectable(label.c_str(), index == selectedNodeType, 0, selectSize)){
				nodesToCreate[type] += 1;
			}
			if((c < columnCount - 1) && (index + columnItemsHeight) < visibleTypesCount){
				ImGui::SameLine((c + 1) * columnWidth);
			}
		}
	}

	if(ImGui::IsKeyReleased(ImGuiKey_Enter)){
		if(selectedNodeType >= 0 && selectedNodeType < int(visibleNodeTypes.size())){
			nodesToCreate[visibleNodeTypes[selectedNodeType]] += 1;
			ImGui::CloseCurrentPopup();
		}
	}

	if(ImGui::IsKeyReleased(ImGuiKey_Escape)){
		ImGui::CloseCurrentPopup();
	}
}

bool editGraph(const std::unique_ptr<Graph>& graph, const std::unordered_map<NodeClass, uint>& nodesToCreate, int nodeToPurgeLinks, const Styling& style, ImVec2& mouseRightClick, std::vector<Node *>& createdNodes) {

	const bool shftModifierHeld = ImGui::GetIO().KeyShift;
	const bool altModifierHeld  = ImGui::GetIO().KeyAlt;
#ifdef _MACOS
	const bool ctrlModifierHeld = ImGui::GetIO().KeySuper;
#else
	const bool ctrlModifierHeld = ImGui::GetIO().KeyCtrl;
#endif

	bool editedGraph = false;

	GraphEditor editor(*graph);
	if(nodeToPurgeLinks >= 0){
		const Node* node = graph->node(nodeToPurgeLinks);
		const uint newInputCount = node->inputs().size();
		const uint newOutputCount = node->outputs().size();
		for(uint i = 0; i < graph->getLinkCount(); ++i){
			const Graph::Link& link = graph->link(i);
			if(int(link.from.node) == nodeToPurgeLinks && link.from.slot >= newOutputCount){
				editor.removeLink(i);
			}
			if(int(link.to.node) == nodeToPurgeLinks && link.to.slot >= newInputCount){
				editor.removeLink(i);
			}
		}
		nodeToPurgeLinks = -1;
	}

	int dropId;
	if(ImNodes::IsLinkDropped(&dropId, false) && altModifierHeld){
		bool isInput = false;
		const Graph::Slot pin = fromLinkToSlot(dropId, isInput);
		if(isInput){
			// Create a constant node and link it.
			Node* createdNode = createNode( shftModifierHeld ? NodeClass::CONST_COLOR : NodeClass::CONST_FLOAT );
			const uint newNodeId = editor.addNode( createdNode );
			// Register for placement at next frame.
			createdNodes.push_back( createdNode );
			mouseRightClick = ImGui::GetMousePos();
			// Shift position to have the end link placed approximately at the mouse.
			mouseRightClick.x -= style.kNodeTotalWidth;
			mouseRightClick.y -= 2.f * ImGui::GetTextLineHeightWithSpacing();

			editor.addLink( newNodeId, 0, pin.node, pin.slot );
			if(shftModifierHeld){
				// Autolink four channels if possible.
				const Node* const toNode = graph->node( pin.node );
				const uint toCount = toNode->inputs().size();
				const uint maxCommonSlots = (std::min)( 4u, toCount - pin.slot );
				for(uint i = 1; i < maxCommonSlots; ++i){
					editor.addLink(newNodeId, i, pin.node, pin.slot + i);
				}
			}
			editedGraph = true;
		}
	}

	int startLink, endLink;
	if(ImNodes::IsLinkCreated(&startLink, &endLink)){
		bool fromIsInput, toIsInput;
		const Graph::Slot from = fromLinkToSlot( startLink, fromIsInput );
		const Graph::Slot to = fromLinkToSlot( endLink, toIsInput );
		assert( toIsInput && !fromIsInput );

		editor.addLink(from.node, from.slot, to.node, to.slot);

		// Multi-link creation
		if(shftModifierHeld){
			const Node* const fromNode = graph->node(from.node);
			const Node* const toNode = graph->node( to.node );
			const uint fromCount = fromNode->outputs().size();
			const uint toCount = toNode->inputs().size();
			const uint maxCommonSlots = (std::min)( fromCount - from.slot, toCount - to.slot );
			for(uint i = 1; i < maxCommonSlots; ++i){
				editor.addLink(from.node, from.slot + i, to.node, to.slot + i);
			}
		}
		editedGraph = true;
	}

	int linkId;
	if(ImNodes::IsLinkDestroyed(&linkId)){

		if(shftModifierHeld){
			const Graph::Link& link = graph->link(linkId);
			const Graph::Slot& from = link.from;
			const Graph::Slot& to = link.to;
			// Find other links between the two nodes, matching channels.
			const Node* const fromNode = graph->node( from.node );
			const Node* const toNode = graph->node( to.node );
			const uint fromCount = fromNode->outputs().size();
			const uint toCount = toNode->inputs().size();
			const uint maxCommonSlots = (std::min)( fromCount - from.slot, toCount - to.slot );
			for( uint i = 1; i < maxCommonSlots; ++i )
			{
				Graph::Link oLink = link;
				oLink.from.slot += i;
				oLink.to.slot += i;
				const int oLinkId = graph->findLink( oLink );
				if( oLinkId >= 0 ){
					editor.removeLink(oLinkId);
				}
			}
		}
		editor.removeLink(linkId);
		editedGraph = true;
	}

	if(ImGui::IsKeyReleased(ImGuiKey_Delete) ||
	   (ImGui::IsKeyReleased(ImGuiKey_Backspace) && ctrlModifierHeld)){
		const uint nodesCount = ImNodes::NumSelectedNodes();
		if(nodesCount > 0u){
			std::vector<int> nodeIds(nodesCount);
			ImNodes::GetSelectedNodes(nodeIds.data());
			for(const int nodeId : nodeIds){
				editor.removeNode((uint)nodeId);
			}
			editedGraph = true;
		}
		const uint linkCount = ImNodes::NumSelectedLinks();
		if(linkCount > 0){
			std::vector<int> linkIds(linkCount);
			ImNodes::GetSelectedLinks(linkIds.data());
			for(const int linkId : linkIds){
				editor.removeLink((uint)linkId);
			}
			editedGraph = true;
		}
	}

	// Create queued nodes.
	for(const auto& nodeTypeToCreate : nodesToCreate){
		if(nodeTypeToCreate.first >= NodeClass::COUNT_EXPOSED){
			continue;
		}
		for(uint i = 0u; i < nodeTypeToCreate.second; ++i){
			Node* createdNode = createNode(nodeTypeToCreate.first);
			editor.addNode( createdNode );
			// Register for placement at next frame.
			createdNodes.push_back(createdNode);
		}
		editedGraph = true;
	}

	editor.commit();
	return editedGraph;
}

bool refreshPreviews(const std::unique_ptr<Graph>& graph, const std::vector<InputFile>& inputFiles, const glm::ivec2& customResolution, bool forceCustomResolution, int previewQuality, bool showAlphaPreview, std::unordered_map<const Node *, GLuint>& textures) {

	CompiledGraph compiledGraph;
	ErrorContext dummyContext;
	bool res = compile(*graph, false, dummyContext, compiledGraph);
	if(!res){
		return false;
	}
	
	// TODO: when errors or unused nodes, do something to give feedback to the user.
	const uint inputCount = compiledGraph.inputs.size();
	// Count selected input files.
	uint inputFileCount = 0u;
	for(const InputFile& input : inputFiles){
		if(!input.active){
			continue;
		}
		++inputFileCount;
	}

	const bool hasEnoughInputsForPreview = inputCount == 0u || (inputFileCount != 0u);
	if(!hasEnoughInputsForPreview){
		return false;
	}

	textures.clear();
	const int previewSize = int(128u / (1u << previewQuality));
	const glm::ivec2 previewRes{previewSize, previewSize};
	// We can evaluate the graph to generate textures.
	// Prepare a batch by hand
	Batch batch;
	// Find the N first selected inputs
	for(const InputFile& input : inputFiles){
		if(!input.active){
			continue;
		}
		batch.inputs.push_back(input.path);
		if(batch.inputs.size() == inputCount){
			break;
		}
	}
	if(!batch.inputs.empty()){
		while(batch.inputs.size() < inputCount){
			// Not enough input images, repeat the last one?
			batch.inputs.push_back(batch.inputs.back());
		}
	}

	// Dummy output names.
	for(uint i = 0; i < compiledGraph.outputs.size(); ++i ){
		Batch::Output& output = batch.outputs.emplace_back();
		output.path = std::to_string(i);
		output.format = Image::Format::PNG;
	}

	SharedContext sharedContext;
	allocateContextForBatch(batch, compiledGraph, customResolution, forceCustomResolution, sharedContext, previewRes);
	for(const CompiledNode& node : compiledGraph.nodes){
		evaluateGraphStepForBatch(node, compiledGraph.stackSize, sharedContext);

		// Use inputs for outputs nodes, to be able to preview the result.
		const bool useInputs = node.outputs.empty();
		const std::vector<Image>& images = useInputs ? sharedContext.tmpImagesRead : sharedContext.tmpImagesWrite;
		const uint texW = images.empty() ? 1 : images[0].w();
		const uint texH = images.empty() ? 1 : images[0].h();
		Image outputImg(texW, texH, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		{
			// Use inputs for outputs nodes, to be able to preview the result.
			const bool useInputs = node.outputs.empty();
			const std::vector<int>& registers = useInputs ? node.inputs : node.outputs;
			const std::vector<Image>& images = useInputs ? sharedContext.tmpImagesRead : sharedContext.tmpImagesWrite;
			const uint channelCount = registers.size();
			// Populate image with available channels from registers.
			for(uint c = 0; c < channelCount; ++c){
				const uint reg = registers[c];
				const Image& img = images[reg/4];
				const uint srcChannel = reg % 4u;
				for(uint y = 0; y < outputImg.h(); ++y){
					for(uint x = 0; x < outputImg.w(); ++x){
						outputImg.pixel(x,y)[c] = img.pixel(x,y)[srcChannel];
					}
				}
			}
			// If we have only one channel, broadcast to RGB, otherwise leave initialized to 0 (or 1 for alpha).
			if(channelCount == 1){
				for(uint y = 0; y < outputImg.h(); ++y){
					for(uint x = 0; x < outputImg.w(); ++x){
						for(uint c = 1; c < 3; ++c){
							outputImg.pixel(x,y)[c] = outputImg.pixel(x,y)[0];
						}
					}
				}
			}
			if(showAlphaPreview){
				for(uint y = 0; y < outputImg.h(); ++y){
					for(uint x = 0; x < outputImg.w(); ++x){
						const float gridLevel = float(((x / 8) % 2) ^ ((y / 8) % 2));
						const glm::vec4 gridColor(gridLevel * 0.5 + 0.25f);
						const float alpha = (outputImg.pixel(x,y)[3]);
						outputImg.pixel(x, y) = glm::mix(gridColor, outputImg.pixel(x, y), alpha);
						outputImg.pixel(x, y)[3] = 1.0f;
					}
				}
			}

		}
		// Upload GL texture and associate to node.
		GLuint tex = 0;
		glGenTextures( 1, &tex );
		glBindTexture( GL_TEXTURE_2D, tex );
		glTexImage2D( GL_TEXTURE_2D, 0,  GL_RGBA32F, outputImg.w(), outputImg.h(), 0, GL_RGBA, GL_FLOAT,  outputImg.rawPixels() );
		glGenerateMipmap( GL_TEXTURE_2D );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glBindTexture( GL_TEXTURE_2D, 0 );
		textures[ node.node ] = tex;

		// And swap for next node.
		std::swap(sharedContext.tmpImagesRead, sharedContext.tmpImagesWrite);
	}
	return true;
}

/// Main loop

int main(int argc, char** argv){
	(void)argc, argv;
	
	Random::seed(743936);

	ImFont* defaultFont = nullptr;
	ImFont* smallFont = nullptr;
	GLFWwindow* window = createWindow(1000, 700, defaultFont, smallFont);
	if(!window){
		Log::Error() << "Unable to create window." << std::endl;
		return 1;
	}

	int winW, winH;
	glfwGetWindowSize(window, &winW, &winH);

	std::unique_ptr<Graph> graph(createDefaultGraph());
	ErrorContext errorContext;

	// Constants
	Styling style;
	style.kSplitBarWidth = 8.0f;
	style.kPinsShape = ImNodesPinShape_CircleFilled;
	style.kPreviewDisplayWidth = 128.f;
	style.kSlotLabelWidth = 24.f;
	style.kNodeInternalWidth = style.kPreviewDisplayWidth + 2.f * style.kSlotLabelWidth;
	style.kNodeTotalWidth = style.kNodeInternalWidth + 2.f * ImNodes::GetStyle().NodePadding.x;
	style.kWinFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar;
	style.defaultFont = defaultFont;
	style.smallFont = smallFont;

	const uint kMaxRefreshDelayInFrames = 60u;
	// Input/output state
	fs::path inputDirectory;
	fs::path outputDirectory;
	std::vector<InputFile> inputFiles;
	uint timeSinceLastInputUpdate = kMaxRefreshDelayInFrames;

	// GUI state
	std::atomic<int> showProgress = -1;
	std::string searchStr;
	std::vector<NodeClass> visibleNodeTypes;
	std::unordered_map<NodeClass, uint> nodesPasteboard;
	std::vector<Node*> createdNodes;
	std::unordered_map<const Node*, GLuint> textures;
	std::unordered_map<const Node*, GLuint> texturesToPurge;
	ImVec2 mouseRightClick( 0.f, 0.f );
	glm::ivec2 customResolution = {64, 64};
	float inputsWindowWidth = (std::min)(300.0f, 0.25f * float(winW));
	int selectedNodeType = 0;
	int seed = Random::getSeed();
	int previewQuality = 1;
	bool showAlphaPreview = true;
	bool showPreview = true;
	bool needsPreviewRefresh = true;
	bool needAutoLayout = true;
	bool anyPopupOpen = false;
	bool forceCustomResolution = false;

	while(!glfwWindowShouldClose(window)) {

		glfwWaitEventsTimeout(0.1);

		// Screen resolution.
		glfwGetWindowSize(window, &winW, &winH);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		purgeTextures(texturesToPurge);

		// Frame state
		bool editedGraph = false;
		bool editedInputList = false;
		bool wantsExit = false;
		int nodeToPurgeLinks = -1;
		bool editingTextField = false;
		style.kSafetyMargin = ImGui::CalcTextSize("M").x;

		// Request exit if pressing escape on main window.
		if(!anyPopupOpen && ImGui::IsKeyReleased(ImGuiKey_Escape)){
			wantsExit = true;
		}
		anyPopupOpen = false;

		// Cyclic input files refresh.
		++timeSinceLastInputUpdate;
		if((timeSinceLastInputUpdate > kMaxRefreshDelayInFrames) && !inputDirectory.empty()){
			editedInputList = refreshFiles(inputDirectory, inputFiles);
			timeSinceLastInputUpdate = 0;
		}

		// Menus and settings
		{
			if(ImGui::BeginMainMenuBar()){

				if(ImGui::BeginMenu("File")){

					if(ImGui::MenuItem("Select directories...")){
						askForDirectories(BOTH, inputDirectory, outputDirectory);
						timeSinceLastInputUpdate = kMaxRefreshDelayInFrames;
					}

					if(ImGui::MenuItem("Change input directory...", nullptr, false, !inputDirectory.empty())){
						askForDirectories(INPUT_IMAGES, inputDirectory, outputDirectory);
						timeSinceLastInputUpdate = kMaxRefreshDelayInFrames;
					}

					if(ImGui::MenuItem("Change output directory...", nullptr, false, !outputDirectory.empty())){
						askForDirectories(OUTPUT_IMAGES, inputDirectory, outputDirectory);
						timeSinceLastInputUpdate = kMaxRefreshDelayInFrames;
					}

					ImGui::Separator();

					if(ImGui::BeginMenu("Settings")){

						if(ImGui::MenuItem("Show preview", "", &showPreview)){
							needsPreviewRefresh = true;
							if(!showPreview){
								texturesToPurge = textures;
								textures.clear();
							}
						}
						if(ImGui::MenuItem("Preview alpha grid", "", &showAlphaPreview)){
							needsPreviewRefresh = true;
						}
						ImGui::PushItemWidth(130);
						if(ImGui::Combo("Preview quality", &previewQuality, "High\0Medium\0Low\0")){
							needsPreviewRefresh = true;
						}
						if(ImGui::InputInt("Random seed", &seed)){
							Random::seed(seed);
						}
						ImGui::PopItemWidth();
						ImGui::EndMenu();
					}

					ImGui::Separator();
					if(ImGui::MenuItem("Quit")){
						wantsExit = true;
					}
					ImGui::EndMenu();
				}

				if(ImGui::BeginMenu("Graph")){

					if(ImGui::MenuItem("Open...")){
						editedGraph = loadGraph(graph, errorContext);
					}

					if(ImGui::MenuItem("Save...", nullptr, false, graph != nullptr)){
						saveGraph(graph, errorContext);
					}

					ImGui::Separator();

					if(ImGui::MenuItem("Validate graph")){
						validate(*graph, errorContext);
					}
					
					if(ImGui::MenuItem( "Run graph" )){
						const std::vector<fs::path> inputPaths = filterInputFiles(inputFiles);
						evaluateInBackground(*graph, errorContext, inputPaths, outputDirectory, customResolution, forceCustomResolution, showProgress);
					}

					ImGui::Separator();

					if(ImGui::MenuItem("Reset...")){
						errorContext.clear();
						// Force the indices free list to purge first.
						graph.reset(nullptr);
						graph.reset(createDefaultGraph());
						needAutoLayout = true;
						editedGraph = true;
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
		
		// Has to be computed after the menu bar.
		const float menuBarHeight = ImGui::GetItemRectSize().y;
		const float editorWindowHeight = float(winH) - menuBarHeight;
		float editorWindowWidth = float(winW) - inputsWindowWidth - style.kSplitBarWidth - 2.f * ImGui::GetStyle().FramePadding.x;

		ImGui::SetNextWindowPos(ImVec2(0.0f, menuBarHeight));
		ImGui::SetNextWindowSize(ImVec2(float(winW), editorWindowHeight));

		if(ImGui::Begin("PackoMainWindow", nullptr, style.kWinFlags)){

			ImGui::Splitter(true, style.kSplitBarWidth, &inputsWindowWidth, &editorWindowWidth, 200, 300);

			// Inputs & outputs panel
			{
				ImGui::BeginChild("Inputs & Outputs", ImVec2(inputsWindowWidth, 0), true);

				if(ImGui::Button("Run")){
					const std::vector<fs::path> inputPaths = filterInputFiles(inputFiles);
					evaluateInBackground(*graph, errorContext, inputPaths, outputDirectory, customResolution, forceCustomResolution, showProgress);
				}

				ImGui::SameLine(inputsWindowWidth - 30.f);
				ImGui::TextDisabled("(?)");
				if(ImGui::IsItemHovered()){
					ImGui::SetTooltip( "Space or Right click: open node palette\nAlt+click: pan around\nWhen dragging from a pin:\n Ctrl: delete link\n Shift: create multiple consecutive links if possible\n Alt: automatically create a constant input node");
				}

				if(showProgress >= 0){
					ImGui::ProgressBar(float(showProgress) / float(kProgressCostGranularity));
				}
				ImGui::Separator();

				ImGui::Text( "Output:" ); ImGui::SameLine();
				if(ImGui::SmallButton("Select...##output")){
					askForDirectories(OUTPUT_IMAGES, inputDirectory, outputDirectory);
				}
				const std::string outputDirStr = outputDirectory.string();
				ImGui::TextWrapped( "%s", outputDirStr.c_str() );

				editedInputList |= ImGui::Checkbox("Custom resolution", &forceCustomResolution);
				if(forceCustomResolution){
					if(ImGui::InputInt2("##res", &customResolution[0])){
						customResolution = glm::max(customResolution, {4, 4});
						editedInputList = true;
					}
				}
				ImGui::Separator();

				ImGui::Text( "Input:" ); ImGui::SameLine(); 
				if(ImGui::SmallButton("Select...##input")){
					askForDirectories(INPUT_IMAGES, inputDirectory, outputDirectory);
					timeSinceLastInputUpdate = kMaxRefreshDelayInFrames;
					editedInputList = true;
				}
				const std::string inputDirStr = inputDirectory.string();
				ImGui::TextWrapped( "%s", inputDirStr.c_str());

				if(ImGui::Button("Select all")){
					for(InputFile& file : inputFiles){
						file.active = true;
					}
					editedInputList = true;
				}
				ImGui::SameLine();
				if(ImGui::Button("Select none")){
					for(InputFile& file : inputFiles){
						file.active = false;
					}
					editedInputList = true;
				}

				if(ImGui::BeginTable( "##Inputs", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg)){
					ImGui::TableSetupColumn( "##bullet", ImGuiTableColumnFlags_WidthFixed, 12 );
					ImGui::TableSetupColumn("Name");
					ImGui::TableHeadersRow();
					for( uint i = 0; i < inputFiles.size(); ++i ){
						ImGui::PushID(i);

						ImGui::TableNextColumn();
						if( ImGui::Selectable("##selec", &inputFiles[i].active, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0, 13))){
							editedInputList = true;
						}
						ImGui::SameLine(-5,0);
						if( inputFiles[ i ].active )
							ImGui::Bullet();

						ImGui::TableNextColumn();
						const std::string filename = inputFiles[ i ].path.filename().string();
						ImGui::Text( "%s", filename.c_str() );

						ImGui::PopID();

					}

					ImGui::EndTable();
				}
				ImGui::EndChild();
			}

			ImGui::SameLine();

			// Editor
			{
				ImGui::BeginChild( "Editor", ImVec2( editorWindowWidth, 0 ) );

				// Graph viewer.
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);

					ImNodes::BeginNodeEditor();

					// Allowing unplugging of links conflicts with multiple outputs
					ImNodes::PushAttributeFlag( 0 /*ImNodesAttributeFlags_EnableLinkDetachWithDragClick*/ );

					// First, immediately register the position for newly created nodes if there are some.
					for(uint nodeId = 0; nodeId < createdNodes.size(); ++nodeId){
						int createdNodeIndex = graph->findNode( createdNodes[nodeId] );
						if( createdNodeIndex >= 0 ){
							const float delta = float( nodeId ) * 20.f;
							const ImVec2 position( mouseRightClick.x + delta, mouseRightClick.y + delta );
							ImNodes::SetNodeScreenSpacePos( createdNodeIndex, position );
						}
					}
					createdNodes.clear();

					GraphNodes nodes(*graph);
					for(const uint nodeId : nodes){
						Node* node = graph->node(nodeId);
						// Very specific case for comments
						if( node->type() == NodeClass::COMMENT ){
							if(drawCommentNode(node, nodeId, style)){
								editingTextField = true;
							}
							continue;
						}
						// Styling for erroring nodes.
						const bool nodeHasIssue = errorContext.contains(node);
						if(nodeHasIssue){
							applyNodeErrorStyle();
						}
						// Standard node.
						editedGraph |= drawNode(node, nodeId, style, textures, nodeToPurgeLinks);
						if( nodeHasIssue ){
							reverseNodeErrorStyle();
						}
					}

					uint linkCount = graph->getLinkCount();
					for(uint linkId = 0u; linkId < linkCount; ++linkId ){
						const Graph::Link& link = graph->link( linkId );
						ImNodes::Link(linkId, fromOutputSlotToLink(link.from), fromInputSlotToLink(link.to));
					}

					ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopRight);
					ImNodes::PopAttributeFlag();
					ImNodes::EndNodeEditor();

					ImGui::PopStyleVar();

					if( needAutoLayout ){
						autoLayout( *graph );
						needAutoLayout = false;
					}
				}
				// List of copied nodes to create if pasting.
				std::unordered_map<NodeClass, uint> nodesToCreate;

				// Creation palette.
				{
					bool focusTextField = false;
					const bool canOpenPopup = !ImGui::IsPopupOpen( "Create node" );
					const bool clickedForPopup = ImGui::IsMouseClicked( ImGuiMouseButton_Right );
					const bool typedForPopup = !editingTextField && ImGui::IsKeyReleased( ImGuiKey_Space );
					if(canOpenPopup && (clickedForPopup || typedForPopup))
					{
						ImGui::OpenPopup("Create node");
						// Save position for placing the new node on screen.
						mouseRightClick = ImGui::GetMousePos();
						focusTextField = true;
						// All types visible by default
						searchStr = "";
						selectedNodeType = 0;
						visibleNodeTypes.resize(NodeClass::COUNT_EXPOSED);
						for(uint i = 0; i < NodeClass::COUNT_EXPOSED; ++i){
							visibleNodeTypes[i] = getOrderedType(i);
						}
					}

					if(ImGui::BeginPopup("Create node")){
						anyPopupOpen = true;
						showCreationPopup(nodesToCreate, searchStr, selectedNodeType, visibleNodeTypes, focusTextField);
						ImGui::EndPopup();
					}
				}

				// Ctrl-C/Ctrl-V
				{
#ifdef _MACOS
					const bool ctrlModifierHeld = ImGui::GetIO().KeySuper;
#else
					const bool ctrlModifierHeld = ImGui::GetIO().KeyCtrl;
#endif
					// Add nodes to the pasteboard when copying.
					if(ImGui::IsKeyReleased(ImGuiKey_C) && ctrlModifierHeld){
						const uint nodesCount = ImNodes::NumSelectedNodes();
						if(nodesCount > 0u){
							nodesPasteboard.clear();

							std::vector<int> nodeIds(nodesCount);
							ImNodes::GetSelectedNodes(nodeIds.data());
							for(const int nodeId : nodeIds){
								const Node* const node = graph->node(nodeId);
								if(node){
									const NodeClass type = NodeClass(node->type());
									nodesPasteboard[type] += 1u;
								}
							}
						}
					}

					if(ImGui::IsKeyReleased(ImGuiKey_V) && ctrlModifierHeld){
						// Save position for placing the new node on screen.
						mouseRightClick = ImGui::GetMousePos();
						for(const auto& nodeTypeCount : nodesPasteboard){
							nodesToCreate[nodeTypeCount.first] += nodeTypeCount.second;
						}
					}
				}

				// Graph edition based on nodes to create and editor actions.
				editedGraph |= editGraph(graph, nodesToCreate, nodeToPurgeLinks, style, mouseRightClick, createdNodes);

				ImGui::EndChild();
			}
		}

		const float totalWindowHeight = ImGui::GetWindowHeight();
		const float totalWindowWidth = ImGui::GetWindowWidth();
		ImGui::End();

		// Clear errors when the graph or inputs have been modified.
		if(editedGraph || editedInputList){
			errorContext.clear();
		}

		// Error panel.
		if(errorContext.hasErrors()){
			// Bottom right corner.
			const Node* errorNodeToFocus = showErrorPanel(errorContext, totalWindowWidth, totalWindowHeight + menuBarHeight);
			if(errorNodeToFocus){
				uint nodeId = graph->findNode( errorNodeToFocus );
				ImNodes::EditorContextMoveToNode( nodeId );
				ImVec2 pan = ImNodes::EditorContextGetPanning();
				ImVec2 size = ImNodes::GetNodeDimensions(nodeId);
				pan.x += 0.5f * (editorWindowWidth - size.x);
				pan.y += 0.5f * (editorWindowHeight - size.y);
				ImNodes::EditorContextResetPanning( pan );
			}
		}

		needsPreviewRefresh |= editedGraph || editedInputList;

		if(needsPreviewRefresh && showPreview){
			// Defer purge by one frame because ImGui is keeping a reference to it for the current frame (partial evaluation?).
			texturesToPurge = textures;
			if(!refreshPreviews(graph, inputFiles, customResolution, forceCustomResolution, previewQuality, showAlphaPreview, textures)){
				// This failed, don't purge.
				textures = texturesToPurge;
				texturesToPurge.clear();
			}
		}
		needsPreviewRefresh = false;

		// We *might* want to exit, ask the user for confirmation.
		if(wantsExit){
			ImGui::OpenPopup("Exit");
		}

		// Exit confirmation popup
		if(ImGui::BeginPopupModal("Exit", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)){
			anyPopupOpen = true;
			ImGui::Text("Are you sure you want to quit?");
			const ImVec2 buttonSize(150.0f, 0.0f);
			if(ImGui::Button("No", buttonSize)){
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
			if(ImGui::IsKeyReleased(ImGuiKey_Backspace)){
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		// Render the interface.
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}
	// Signal threads that we want to exit.
	showProgress = kProgressImmediateStop;

	// Wait for any progressing background task to complete.
	while(showProgress >= 0 && showProgress < kProgressImmediateStop){
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	// Purge GL texture pool.
	purgeTextures( textures );

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
