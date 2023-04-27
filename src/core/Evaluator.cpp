#include "core/Evaluator.hpp"
#include "core/Graph.hpp"
#include "core/nodes/Nodes.hpp"
#include "core/Image.hpp"
#include "core/system/System.hpp"

#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <deque>
#include <chrono>

#define PARALLEL_FOR


void ErrorContext::addError(const std::string& message, const Node* node, int slot){
	_errors.emplace_back(message, node, slot);
	_dirtySummary = true;
}

std::string ErrorContext::summarizeErrors(){
	if(_dirtySummary){

		std::stringstream str;
		str << "Graph validation: " << _errors.size() << " errors encountered." << "\n";
		for(const Error& error : _errors){
			str << "* " << "Error";
			if(error.node){
				str << " for node " << error.node->name();
			}
			if(error.slot >= 0){
				str << " for slot " << error.slot;
			}
			str << ": " << error.message << "\n";
		}

		_cachedSummary = str.str();
		_dirtySummary = false;
	}
	return _cachedSummary;
}

void ErrorContext::getError( uint i, const char*& message, const Node*& node, int& slot )
{
	assert( i < _errors.size() );
	Error& error = _errors[ i ];
	message = error.message.c_str();
	node = error.node;
	slot = error.slot;
}

void ErrorContext::clear(){
	_errors.clear();
	_dirtySummary = true;
}

bool ErrorContext::contains( const Node* const node ) const
{
	for( const Error& error : _errors )
	{
		if( error.node == node )
			return true;
	}
	return false;
}



class WorkGraph {
public:
	struct Vertex;

	struct Edge {
		uint from;
		uint to;
	};

	struct Neighbor {
		Vertex* node;
		std::vector<Edge> edges;

		Neighbor(Vertex* _node, uint from, uint to) : node(_node) {
			edges.push_back({from, to});
		}

		void addEdge(uint from, uint to){
			edges.push_back({from, to});
		}
	};

	struct Vertex {
		const Node* node;
		std::vector<Neighbor> children;
		std::vector<Neighbor> parents;

		uint tmpData{0u};

		Vertex(const Node* _node) : node(_node){}
	};

	WorkGraph(const Graph& graph, ErrorContext& errorContext) : _context(errorContext){

		GraphNodes nodesIt(graph);
		// Allocate nodes.
		for(uint node : nodesIt){
			_nodesPool.emplace_back(graph.node(node));
		}
		// Insert working nodes.
		for(Vertex& node : _nodesPool){
			nodes.push_back(&node);
		}

		uint linkCount = graph.getLinkCount();
		for(uint lid = 0; lid < linkCount; ++lid){
			const Graph::Link& link = graph.link(lid);
			const Node* linkFrom = graph.node(link.from.node);
			const Node* linkTo = graph.node(link.to.node);

			auto fromNode = std::find_if(nodes.begin(), nodes.end(), [linkFrom](const Vertex* const node){
				return node->node == linkFrom;
			});
			auto toNode = std::find_if(nodes.begin(), nodes.end(), [linkTo](const Vertex* const node){
				return node->node == linkTo;
			});

			if(fromNode == nodes.end() || toNode == nodes.end()){
				// Incorrect link
				continue;
			}

			std::vector<Neighbor>& fromChildren = (*fromNode)->children;
			auto child = std::find_if(fromChildren.begin(), fromChildren.end(), [&toNode](const Neighbor& child){
				return child.node == *toNode;
			});
			if(child == fromChildren.end()){
				fromChildren.emplace_back(*toNode, link.from.slot, link.to.slot);
			} else {
				child->addEdge( link.from.slot, link.to.slot);
			}

			std::vector<Neighbor>& toParents = (*toNode)->parents;
			auto parent = std::find_if(toParents.begin(), toParents.end(), [&fromNode](const Neighbor& parent){
				return parent.node == *fromNode;
			});

			if(parent == toParents.end()){
				toParents.emplace_back(*fromNode, link.from.slot, link.to.slot);
			} else {
				parent->addEdge( link.from.slot, link.to.slot);
			}

		}
	}

	bool validateInputs(){
		bool incompleteNodes = false;
		// Check that all nodes have their inputs filled.
		for(uint nid = 0; nid < nodes.size();){
			const Vertex* node = nodes[nid];
			const uint tgtSlotCount = node->node->inputs().size();

			// Check that each slot is assigned to one of the parents.
			for(uint sid = 0; sid < tgtSlotCount; ++sid){
				bool foundSlot = false;
				// Find the parent.
				for(const Neighbor& neigh : node->parents){
					for(const Edge& edge : neigh.edges){
						if(edge.to == sid){
							foundSlot = true;
							break;
						}
					}
					if(foundSlot){
						break;
					}
				}
				if(!foundSlot){
					_context.addError("Missing input", node->node, sid);
					incompleteNodes = true;
				}
			}
			++nid;
		}
		return !incompleteNodes;
	}

	void purgeTmpData(){
		for(Vertex* node : nodes){
			node->tmpData = 0u;
		}
	}

	bool hasCycle(Vertex* node){
		// Have we already visited the node.
		if(node->tmpData != 0u){
			_context.addError("Cycle detected", node->node);
			return true;
		}
		// Mark as visited
		node->tmpData = 1u;
		bool childHasCycle = false;
		for(const Neighbor& child : node->children){
			childHasCycle |= hasCycle(child.node);
		}
		// Unmark
		node->tmpData = 0u;
		return childHasCycle;
	}

	bool validateCycles(){
		purgeTmpData();

		// Collect roots.
		std::vector<Vertex*> roots;
		for(Vertex* node : nodes){
			if(node->parents.empty()){
				roots.push_back(node);
			}
		}
		bool rootHasCycle = false;
		for(Vertex* root : roots){
			rootHasCycle |= hasCycle(root);
		}
		return !rootHasCycle;
	}

	bool validateOutputNames(){
		bool duplicated = false;
		std::unordered_map<std::string, const OutputNode*> outputNames;
		for(const Vertex* node : nodes){
			if(node->node->type() != NodeClass::OUTPUT_IMG){
				continue;
			}
			// Generate the filename
			const OutputNode* outNode = static_cast<const OutputNode*>(node->node);
			Image::Format tmpFormat;
			const std::string filename = outNode->generateFileName(0, tmpFormat);
			auto other = outputNames.find(filename);
			if(other != outputNames.end()){
				duplicated = true;
				_context.addError("Colliding output name with " + (*other).second->name(), outNode);
			} else {
				outputNames[filename] = outNode;
			}
		}

		return !duplicated;
	}

	void checkConnectionToOutputs(Vertex* vertex){
		if(vertex->tmpData == 1u){
			return;
		}

		if(vertex->node->type() == NodeClass::OUTPUT_IMG ){
			vertex->tmpData = 1u;
			return;
		}

		bool connected = false;
		for(Neighbor& child : vertex->children){
			checkConnectionToOutputs(child.node);
			connected |= child.node->tmpData == 1u;
		}
		if(connected){
			vertex->tmpData = 1u;
		}
	}

	void cleanUnconnectedComponents(){
		purgeTmpData();
		// Collect roots.
		std::vector<Vertex*> roots;
		for(Vertex* node : nodes){
			if(node->parents.empty()){
				roots.push_back(node);
			}
		}
		for(Vertex* node : roots){
			checkConnectionToOutputs(node);
		}
		for(Vertex* vert : nodes){
			auto itp = std::remove_if(vert->parents.begin(), vert->parents.end(), [](const Neighbor& parent){
				return parent.node->tmpData == 0u;
			});
			vert->parents.erase( itp, vert->parents.end() );
			auto itc = std::remove_if(vert->children.begin(), vert->children.end(), [](const Neighbor& child){
				return child.node->tmpData == 0u;
			});
			vert->children.erase( itc, vert->children.end() );

		}
		auto itn = std::remove_if(nodes.begin(), nodes.end(), [](const Vertex* node){
			return node->tmpData == 0u;
		});
		nodes.erase( itn, nodes.end() );
	}

	void compile(CompiledGraph& compiledGraph, bool optimize){
		std::vector<Vertex*> orderedNodes;
		orderedNodes.reserve(nodes.size());
		purgeTmpData();
		// Collect roots.
		std::deque<Vertex*> nodesToProcess;
		for(Vertex* node : nodes){
			if(node->parents.empty()){
				nodesToProcess.push_back(node);
			}
		}
		//
		while(!nodesToProcess.empty()){
			Vertex* top = nodesToProcess.front();
			nodesToProcess.pop_front();
			if(top->tmpData == 1u)
				continue;

			bool parentsReady = true;
			for(Neighbor& parent : top->parents){
				if(parent.node->tmpData == 0u){
					parentsReady = false;
					break;
				}
			}
			// Skip if not ready yet.
			if(!parentsReady){
				nodesToProcess.push_back(top);
				continue;
			}
			// The node can be processed, mark as complete.
			orderedNodes.push_back(top);
			top->tmpData = 1u;
			// Insert all children
			for(Neighbor& child : top->children){
				// If a child is already processed, nothing to do, its own children
				// have already been enqueued.
				if(child.node->tmpData == 1u)
					continue;
				// Put them at the front to preserve consecutive nodes as much as possible.
				nodesToProcess.push_front(child.node);
			}
		}
		nodes = orderedNodes;
		purgeTmpData();

		const uint nodeCount = nodes.size();
		std::vector<std::vector<uint>> registersRefCount(nodeCount);

		std::vector<CompiledNode>& compiledNodes = compiledGraph.nodes;
		compiledNodes.resize(nodeCount);

		for (uint nId = 0u; nId < nodeCount; ++nId) {
			Vertex* vert = nodes[nId];
			// Having indices will make things easier later on
			vert->tmpData = nId;
			compiledNodes[nId].node = vert->node;
			// Initialize registers
			const uint inputCount = vert->node->inputs().size();
			const uint outputCount = vert->node->outputs().size();
			compiledNodes[nId].inputs.resize(inputCount, -1);
			compiledNodes[nId].outputs.resize(outputCount, -1);
			// Initialize ref count based on childrens using each output slot.
			registersRefCount[nId].resize(outputCount, 0);
			for (Neighbor& child : vert->children) {
				for (Edge& edge : child.edges) {
					++(registersRefCount[nId][edge.from]);
				}
			}
		}

		FreeList availableRegisters;
		int maxRegister = -1;

		for(Vertex* vert : nodes){
			CompiledNode& compiledNode = compiledNodes[vert->tmpData];
			// Retrieve registers used by parents for their outputs.
			for(Neighbor& parent : vert->parents){
				CompiledNode& compiledParent = compiledNodes[parent.node->tmpData];
				for(Edge& edge : parent.edges){
					// A parent must have been processed before the child node.
					const int linkRegister = compiledParent.outputs[edge.from];
					if (linkRegister < 0) {
						_context.addError("Unassigned output node.", parent.node->node, edge.from);
					}
					compiledNode.inputs[edge.to] = linkRegister;

					// We want to return registers from parents that have propagated them to all their children.
					--registersRefCount[parent.node->tmpData][edge.from];
					if(registersRefCount[parent.node->tmpData][edge.from] == 0u){
						availableRegisters.returnIndex(linkRegister);
					}
				}

			}
			// Assign registers to children.
			for(Neighbor& child : vert->children){
				for(Edge& edge : child.edges){
					// Skip outputs that have already been assigned when processing a previous child.
					if(compiledNode.outputs[edge.from] == -1){
						const uint newRegister = availableRegisters.getIndex();
						compiledNode.outputs[edge.from] = newRegister;
						maxRegister = (std::max)(maxRegister, int(newRegister));
					}
				}
			}

		}
		uint stackSize = (uint)(maxRegister + 1);
		const int firstDummyRegister = stackSize;
		const uint countDummyRegister = optimize ? 1u : 4u;
		compiledGraph.stackSize = stackSize + countDummyRegister;
		compiledGraph.firstDummyRegister = stackSize;

		for (CompiledNode& node : compiledGraph.nodes) {
			// Safety check.
			uint slotId = 0u;
			for (int reg : node.inputs) {
				if (reg < 0) {
					_context.addError("Unassigned input node.", node.node, slotId);
				}
				++slotId;
			}
			uint currentDummyRegister = firstDummyRegister;
			for (int& reg : node.outputs) {
				if(reg == -1){
					reg = currentDummyRegister;
					currentDummyRegister = (std::min)(currentDummyRegister + 1u, compiledGraph.stackSize - 1);
				}
			}
		}

		// By default, assume we'll want to store all registers in image channels.
		compiledGraph.tmpImageCount = (compiledGraph.stackSize + 3u)/4u;
		// Refresh in/out nodes.
		compiledGraph.collectInputsAndOutputs();
	}

	std::vector<Vertex*> nodes;

private:

	ErrorContext& _context;

	std::vector<Vertex> _nodesPool;

};

void CompiledGraph::ensureGlobalNodesConsistency(){
	// Find all flush nodes, find all "in transit" registers, assign them images.
	// Then when executing, before each "flush" node have a "backup" node and afterwards a "restore" node.
	struct Split {
		uint index;
		std::unordered_set<uint> redirections;
	};
	std::vector<Split> splits;
	std::unordered_set<uint> registersInFlight;

	const int nodeCount = int(nodes.size());
	for(int i = nodeCount - 1; i >= 0; --i){
		const CompiledNode& compiledNode = nodes[i];
		// Remove in flight registers provided by this node.
		for(int index : compiledNode.outputs){
			registersInFlight.erase(index);
		}
		// If the node is global, list it.
		if(compiledNode.node->global()){
			splits.insert(splits.begin(), {uint(i), registersInFlight});
		}
		// Inputs need to be provided if there is a flush node before.
		for(int index : compiledNode.inputs){
			if(index < firstDummyRegister){
				registersInFlight.insert(index);
			}
		}
	}

	uint totalShift = 0u;
	uint maxTmpChannelCount = 0u;
	for(Split& split : splits){
		const uint backupIndex  = split.index + totalShift;
		const uint globalIndex  = backupIndex + 1u;
		const uint restoreIndex = globalIndex + 1u;
		nodes.emplace(nodes.begin() + backupIndex);
		nodes.emplace(nodes.begin() + restoreIndex);
		split.index = globalIndex;
		totalShift += 2u;

		CompiledNode& backup  = nodes[backupIndex];
		CompiledNode& global  = nodes[globalIndex];
		CompiledNode& restore = nodes[restoreIndex];
		backup.node = new BackupNode();
		restore.node = new RestoreNode();

		// Backup nodesetup
		{
			// First put all registers used by the global node as inputs.
			backup.inputs = global.inputs;
			// Then all others registers to preserve if not already inserted
			for(uint redir : split.redirections){
				if(std::find(backup.inputs.begin(), backup.inputs.end(), redir) == backup.inputs.end()){
					backup.inputs.push_back(redir);
				}
			}
			// Move each register to an image channel, in order.
			const uint backupInputCount = backup.inputs.size();
			backup.outputs.resize(backupInputCount);
			for(uint i = 0; i < backupInputCount; ++i){
				backup.outputs[i] = i;
			}
		}
		// Global node adjustments
		{
			// The global node will have access to all the backed up images.
			const uint nodeInputCount = global.inputs.size();
			for(uint i = 0; i < nodeInputCount; ++i){
				global.inputs[i] = i;
			}
			// The global node will directly write its outputs to the registers for each pixel.
			// As long as the operation is a gathering.
		}

		// Restore node setup
		{
			restore.inputs = backup.outputs;
			restore.outputs = backup.inputs;
			// Ignore the ones that are output by the global node, by directing them to the dummy register.
			for(int& restored : restore.outputs){
				if(std::find(global.outputs.begin(), global.outputs.end(), restored) != global.outputs.end()){
					restored = firstDummyRegister;
				}
			}
		}

		// Number of backup channels needed.
		maxTmpChannelCount = (std::max)(maxTmpChannelCount, uint(backup.inputs.size()));
	}
	tmpImageCount = (maxTmpChannelCount + 3u) / 4u;
}

void CompiledGraph::collectInputsAndOutputs(){

	inputs.clear();
	outputs.clear();

	for(const CompiledNode& compiledNode : nodes){
		if(compiledNode.node->type() == NodeClass::INPUT_IMG){
			inputs.push_back(compiledNode.node);
		}
		if(compiledNode.node->type() == NodeClass::OUTPUT_IMG){
			outputs.push_back(compiledNode.node);
		}
	}

	auto sortByName = [](const Node* a, const Node* b){
		return a->name() < b->name();
	};

	std::sort(inputs.begin(), inputs.end(), sortByName);
	std::sort(outputs.begin(), outputs.end(), sortByName);
}

void CompiledGraph::clearInternalNodes(){
	for(CompiledNode& node : nodes){
		if(node.node && node.node->type() >= NodeClass::COUNT_EXPOSED){
			delete node.node;
			node.node = nullptr;
		}
	}
}

CompiledGraph::~CompiledGraph(){
	clearInternalNodes();
}

bool generateBatches(const std::vector<const Node*>& inputs, const std::vector<const Node*>& outputs, const std::vector<fs::path>& inputPaths, const fs::path& outputPath, std::vector<Batch>& batches){

	// Generate batches based on the number of file nodes and paths given.
	const uint inputCount = inputs.size();
	const uint outputCount = outputs.size();
	const uint pathCount = inputPaths.size();
	const uint batchCount = std::max(pathCount, 1u) / std::max(inputCount, 1u);

	// Check that we have the correct number of input paths
	if(batchCount * std::max(inputCount, 1u) != std::max(pathCount, 1u)){
		return false;
	}
	batches.resize(batchCount);

	// Prepare input and output paths.
	for(uint batchId = 0u; batchId < batchCount; ++batchId){
		Batch& batch = batches[batchId];
		for(uint inputId = 0u; inputId < inputCount; ++inputId){
			const uint pathId = batchId * inputCount + inputId;
			assert(pathId < pathCount);
			batch.inputs.push_back(inputPaths[pathId]);
		}
		for(uint outputId = 0u; outputId < outputCount; ++outputId){
			const OutputNode* node = static_cast<const OutputNode*>(outputs[outputId]);
			Batch::Output& outFile = batch.outputs.emplace_back();
			outFile.path = outputPath / node->generateFileName(batchId, outFile.format);
		}
	}
	return true;
}

glm::ivec2 computeOutputResolution(const std::vector<Image>& images, const glm::ivec2& fallbackRes)
{
	if(images.empty()){
		return fallbackRes;
	}

	glm::ivec2 tgtRes{INT_MAX, INT_MAX };
	for(const Image& img : images){
		tgtRes = glm::min(tgtRes, {img.w(), img.h() });
	}
	return tgtRes;
}

bool validate(WorkGraph& graph){
	bool success = true;

	if(!graph.validateInputs()){
		success = false;
	}
	if(!graph.validateCycles()){
		success = false;
	}
	if(!graph.validateOutputNames()){
		success = false;
	}
	return success;
}

bool validate(const Graph& editGraph, ErrorContext& errors ){

	errors.clear();

	WorkGraph graph(editGraph, errors);

	return validate(graph);
}

bool compile( const Graph& editGraph, bool optimize, ErrorContext& errors, CompiledGraph& compiledGraph ){

	errors.clear();

	// Validate the graph.
	WorkGraph graph( editGraph, errors );

	if( !validate( graph ) ){
		return false;
	}
	// We dont have incomplete nodes anymore.
	// We could have unconnected regions.
	if(optimize){
		graph.cleanUnconnectedComponents();
	}

	// Compile the graph for real.
	graph.compile( compiledGraph, optimize );
	// Don't alter global nodes. (or boolean?)

	if(optimize){
		compiledGraph.ensureGlobalNodesConsistency();
	}
	return true;
}

void allocateContextForBatch(const Batch& batch, const CompiledGraph& compiledGraph, const glm::ivec2& fallbackRes, bool forceRes, SharedContext& sharedContext){

	const uint inputCountInBatch  = batch.inputs.size();
	const uint outputCountInBatch = batch.outputs.size();

	sharedContext.inputImages.resize(inputCountInBatch);
	for(uint i = 0u; i < inputCountInBatch; ++i){
		sharedContext.inputImages[i].load(batch.inputs[i]);
	}
	// Check that all images have the same size.
	const glm::ivec2 minRes = computeOutputResolution( sharedContext.inputImages, fallbackRes );
	sharedContext.dims = minRes;
	sharedContext.scale = { 1.f, 1.f };

	if(forceRes){
		sharedContext.dims = fallbackRes;
		sharedContext.scale = glm::vec2( fallbackRes ) / glm::max( glm::vec2( minRes ), { 1.f, 1.f } );

		for(uint i = 0u; i < inputCountInBatch; ++i){
			sharedContext.inputImages[i].resize(sharedContext.dims);
		}
	}
	// Allocate outputs
	const uint w = sharedContext.dims.x;
	const uint h = sharedContext.dims.y;
	for(uint i = 0u; i < outputCountInBatch; ++i){
		sharedContext.outputImages.emplace_back(w, h);
	}
	// Allocate tmp images
	for(uint i = 0u; i < compiledGraph.tmpImageCount; ++i){
		sharedContext.tmpImagesRead.emplace_back(w, h);
		sharedContext.tmpImagesWrite.emplace_back(w, h);
	}
}

void evaluateGraphStepForBatch(const CompiledNode& compiledNode, uint stackSize, SharedContext& sharedContext){

	const uint w = sharedContext.dims.x;
	const uint h = sharedContext.dims.y;

#ifdef PARALLEL_FOR
	System::forParallel(0, h, [&sharedContext, w, &compiledNode, stackSize](size_t y){
#else
	for( uint y = 0; y < h; ++y ){
#endif
		for( uint x = 0; x < w; ++x ){
			// Create local context (shared context + x,y coords and a scratch space)
			LocalContext context(&sharedContext, {x,y}, stackSize);
			// Transfer inputs to registers
			for(uint sid = 0; sid < stackSize; ++sid){
				context.stack[sid] = context.shared->tmpImagesRead[sid/4].pixel(x,y)[sid%4];
			}
			// Run the compiled graph, assigning to registers, passing the context along.
			compiledNode.node->evaluate(context, compiledNode.inputs, compiledNode.outputs);
			// Transfer registers to outputs
			for(uint sid = 0; sid < stackSize; ++sid){
				context.shared->tmpImagesWrite[sid/4].pixel(x,y)[sid%4] = context.stack[sid];
			}
		}
	}
#ifdef PARALLEL_FOR
	);
#endif
}

void evaluateGraphForBatchLockstep(const CompiledGraph& compiledGraph, SharedContext& sharedContext){
	const uint nodeCount = compiledGraph.nodes.size();
	for(uint nodeId = 0u; nodeId < nodeCount; ++nodeId){
		evaluateGraphStepForBatch(compiledGraph.nodes[nodeId], compiledGraph.stackSize, sharedContext);
		std::swap(sharedContext.tmpImagesRead, sharedContext.tmpImagesWrite);
	}
}

void evaluateGraphForBatchOptimized(const CompiledGraph& compiledGraph, SharedContext& sharedContext){
	const uint compiledNodeCount = compiledGraph.nodes.size();
	const uint w = sharedContext.dims.x;
	const uint h = sharedContext.dims.y;
	uint currentStartNodeId = 0u;

	std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();

	while(currentStartNodeId < compiledNodeCount){
		// Find the next global node
		uint nextGlobalNodeId = currentStartNodeId + 1;
		for(; nextGlobalNodeId < compiledNodeCount; ++nextGlobalNodeId){
			if(compiledGraph.nodes[ nextGlobalNodeId ].node->global())
				break;
		}
		// Now we have a range [currentStartNode, nextGlobalNodeId[ to execute per-pixel.
#ifdef PARALLEL_FOR
		System::forParallel(0, h, [&sharedContext, currentStartNodeId, nextGlobalNodeId, w, &compiledGraph](size_t y){
#else
		for( uint y = 0; y < h; ++y ){
#endif
			for( uint x = 0; x < w; ++x ){
				// Create local context (shared context + x,y coords and a scratch space)
				LocalContext context(&sharedContext, {x,y}, compiledGraph.stackSize);

				// Run the compiled graph, assigning to registers, passing the context along.
				for(uint nodeId = currentStartNodeId; nodeId < nextGlobalNodeId; ++nodeId){
					const CompiledNode& compiledNode = compiledGraph.nodes[nodeId];
					compiledNode.node->evaluate(context, compiledNode.inputs, compiledNode.outputs);
				}
			}
		}
#ifdef PARALLEL_FOR
		);
#endif

		std::swap(sharedContext.tmpImagesRead, sharedContext.tmpImagesWrite);

		currentStartNodeId = nextGlobalNodeId;
	}

	std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
	const long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	Log::Info() << "Batch took " << duration << "ms." << std::endl;
}

void saveContextForBatch(const Batch& batch, const SharedContext& context){
	// Save outputs
	for (uint i = 0u; i < batch.outputs.size(); ++i) {
		const Batch::Output& output = batch.outputs[i];
		context.outputImages[i].save(output.path, output.format);
	}
}


bool evaluate(const Graph& editGraph, ErrorContext& errors, const std::vector<fs::path>& inputPaths, const fs::path& outputDir, const glm::ivec2& fallbackRes){

	CompiledGraph compiledGraph;
	if(!compile(editGraph, true, errors, compiledGraph)){
		return false;
	}

	// Populate batches with file info.
	std::vector<Batch> batches;

	// Collect file nodes.
	if(!generateBatches( compiledGraph.inputs, compiledGraph.outputs, inputPaths, outputDir, batches)){
		errors.addError("Not enough input files.");
		return false;
	}

	for(const Batch& batch : batches){

		SharedContext sharedContext;
		allocateContextForBatch(batch, compiledGraph, fallbackRes, false, sharedContext);

		evaluateGraphForBatchOptimized(compiledGraph, sharedContext);

		saveContextForBatch(batch, sharedContext);
	}

	return true;
}

bool evaluateInBackground(const Graph& editGraph, ErrorContext& errors, const std::vector<fs::path>& inputPaths, const fs::path& outputDir, const glm::ivec2& fallbackRes, std::atomic<int>& progress){

	CompiledGraph compiledGraph;
	if(!compile(editGraph, true, errors, compiledGraph)){
		return false;
	}

	// Populate batches with file info.
	std::vector<Batch> batches;

	// Collect file nodes.
	if(!generateBatches( compiledGraph.inputs, compiledGraph.outputs, inputPaths, outputDir, batches)){
		errors.addError("Not enough input files.");
		return false;
	}

	// Pass local objects by copy.
	std::thread thread([&progress, compiledGraph, batches, fallbackRes ](){
		progress = 0.f;
		const int batchCost = std::floor(1.f / float(batches.size()) * kProgressCostGranularity);
		for(const Batch& batch : batches){
			if(progress >= kProgressImmediateStop){
				break;
			}
			SharedContext sharedContext;
			allocateContextForBatch(batch, compiledGraph, fallbackRes, false, sharedContext);

			evaluateGraphForBatchOptimized(compiledGraph, sharedContext);

			saveContextForBatch(batch, sharedContext);
			progress += batchCost;
		}
		progress = -1.f;
	});
	thread.detach();

	return true;
}
