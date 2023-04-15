#include "core/Evaluator.hpp"
#include "core/Graph.hpp"
#include "core/nodes/Nodes.hpp"
#include "core/Image.hpp"

#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <deque>

struct NodeAndRegisters { 
	const Node* node;
	std::vector<int> inputs;
	std::vector<int> outputs;
};

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

	uint compile(std::vector<NodeAndRegisters>& compiledNodes ){
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

		std::vector<std::vector<uint>> registersRefCount;
		const uint nodeCount = nodes.size();
		compiledNodes.resize(nodeCount);
		registersRefCount.resize(nodeCount);

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
			NodeAndRegisters& compiledNode = compiledNodes[vert->tmpData];
			// Retrieve registers used by parents for their outputs.
			for(Neighbor& parent : vert->parents){
				NodeAndRegisters& compiledParent = compiledNodes[parent.node->tmpData];
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
		const uint dummyRegister = stackSize;
		++stackSize;

		for (NodeAndRegisters& node : compiledNodes) {
			// Safety check.
			uint slotId = 0u;
			for (int reg : node.inputs) {
				if (reg < 0) {
					_context.addError("Unassigned input node.", node.node, slotId);
				}
				++slotId;
			}
			for (int& reg : node.outputs) {
				if (reg == -1) {
					reg = dummyRegister;
				}
			}
		}
		return stackSize;
	}

	std::vector<Vertex*> nodes;

private:

	ErrorContext& _context;

	std::vector<Vertex> _nodesPool;

};

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

struct Batch {

	struct Output {
		std::string path;
		Image::Format format;
	};

	std::vector<std::string> inputs;
	std::vector<Output> outputs;
};


bool evaluate(const Graph& editGraph, ErrorContext& errors, const std::vector<std::string>& inputPaths, const std::string& outputDir, const glm::ivec2& outputRes ){

	errors.clear();

	// Validate the graph.
	WorkGraph graph(editGraph, errors);

	if(!validate(graph)){
		return false;
	}
	// We dont have incomplete nodes anymore.
	// We could have unconnected regions.
	graph.cleanUnconnectedComponents();

	std::vector<NodeAndRegisters> compiledNodes;
	const uint stackSize = graph.compile(compiledNodes);
	// For now, assume a unique subgraph (no flush), we'll replace flush nodes by virtual inputs/outputs with appended inputs
	// Find all flush nodes, find all "in transit" registers, assign them images
	// Then when executing, before each "flush" node have a "backup" node and afterwards a "restore" node
	// the "flush" node will have 4 inputs and outputs, so we'll have to ensure this 4 are packed in the same image.
	struct Split {
		uint index;
		std::unordered_set<uint> redirections;
	};
	const int dummyRegister = std::max(int(stackSize) - 1, 0);
	std::vector<Split> splits;
	std::unordered_set<uint> registersInFlight;

	const int nodeCount = int(compiledNodes.size());
	for(int i = nodeCount - 1; i >= 0; --i){
		const NodeAndRegisters& compiledNode = compiledNodes[i];
		// Remove in flight registers provided by this node.
		for(int index : compiledNode.outputs){
			registersInFlight.erase(index);
		}
		// If the node is global, list it.
		if(compiledNodes[i].node->global()){
			splits.insert(splits.begin(), {uint(i), registersInFlight});
		}
		// Inputs need to be provided if there is a flush node before.
		for(int index : compiledNode.inputs){
			if(index != dummyRegister){
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
		compiledNodes.emplace(compiledNodes.begin() + backupIndex);
		compiledNodes.emplace(compiledNodes.begin() + restoreIndex);
		split.index = globalIndex;
		totalShift += 2u;

		NodeAndRegisters& backup  = compiledNodes[backupIndex];
		NodeAndRegisters& global  = compiledNodes[globalIndex];
		NodeAndRegisters& restore = compiledNodes[restoreIndex];
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
					restored = dummyRegister;
				}
			}
		}

		// Number of backup channels needed.
		maxTmpChannelCount = (std::max)(maxTmpChannelCount, uint(backup.inputs.size()));
	}
	const uint tmpImageCountInBatch = (maxTmpChannelCount + 3u) / 4u;

	// Collect file nodes.
	std::vector<const Node*> inputs;
	std::vector<const Node*> outputs;
	auto sortByName = [](const Node* a, const Node* b){
		return a->name() < b->name();
	};
	for(const NodeAndRegisters& compiledNode : compiledNodes){
		if(compiledNode.node->type() == NodeClass::INPUT_IMG){
			inputs.push_back(compiledNode.node);
		}
		if(compiledNode.node->type() == NodeClass::OUTPUT_IMG){
			outputs.push_back(compiledNode.node);
		}

		std::sort(inputs.begin(), inputs.end(), sortByName);
		std::sort(outputs.begin(), outputs.end(), sortByName);
	}

	// Generate batches based on the number of file nodes and paths given.

	const uint inputCount = inputs.size();
	const uint outputCount = outputs.size();
	const uint pathCount = inputPaths.size();
	const uint batchCount = std::max(pathCount, 1u) / std::max(inputCount, 1u);

	// Check that we have the correct number of input paths
	if(batchCount * std::max(inputCount, 1u) != std::max(pathCount, 1u)){
		errors.addError("Not enough input files, expected batches of " + std::to_string(inputCount) + " files.");
		return false;
	}

	// Prepare input and output paths.
	std::vector<Batch> batches(batchCount);
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
			outFile.path = outputDir + "/" + node->generateFileName(batchId, outFile.format);
		}
	}

	for(const Batch& batch : batches){
		// TODO: log feedback.

		const uint inputCountInBatch = batch.inputs.size();
		const uint outputCountInBatch = batch.outputs.size();

		// Load inputs
		SharedContext sharedContext;
		sharedContext.inputImages.resize(inputCountInBatch);
		for(uint i = 0u; i < inputCountInBatch; ++i){
			sharedContext.inputImages[i].load(batch.inputs[i]);
		}
		// Check that all images have the same size.
		uint w = outputRes[0]; uint h = outputRes[1];
		if(inputCountInBatch > 0){
			w = sharedContext.inputImages[0].w();
			h = sharedContext.inputImages[0].h();
			for (uint i = 1u; i < inputCount; ++i) {
				w = (std::min)(w, sharedContext.inputImages[i].w());
				h = (std::min)(h, sharedContext.inputImages[i].h());
			}
		}
		sharedContext.dims = { w, h };

		// Create outputs
		for(uint i = 0u; i < outputCountInBatch; ++i){
			sharedContext.outputImages.emplace_back(w, h);
		}
		// Create tmp images
			sharedContext.tmpImages.emplace_back(w, h);
		for(uint i = 0u; i < tmpImageCountInBatch; ++i){
		}

		const uint compiledNodeCount = compiledNodes.size();
		uint currentStartNodeId = 0u;

		while(currentStartNodeId < compiledNodeCount){
			// Find the next global node
			uint nextGlobalNodeId = currentStartNodeId + 1;
			for(; nextGlobalNodeId < compiledNodeCount; ++nextGlobalNodeId){
				if(compiledNodes[ nextGlobalNodeId ].node->global())
					break;
			}
			// Now we have a range [currentStartNode, nextGlobalNodeId[ to execute per-pixel.
			for( uint y = 0; y < h; ++y ){
				for( uint x = 0; x < w; ++x ){
					// Create local context (shared context + x,y coords and a scratch space)
					LocalContext context(&sharedContext, {x,y}, stackSize);

					// Run the compiled graph, assigning to registers, passing the context along.
					for(uint nodeId = currentStartNodeId; nodeId < nextGlobalNodeId; ++nodeId){
						const NodeAndRegisters& compiledNode = compiledNodes[nodeId];
						compiledNode.node->evaluate(context, compiledNode.inputs, compiledNode.outputs);
					}
				}
			}
			currentStartNodeId = nextGlobalNodeId;
		}

		// Save outputs
		for (uint i = 0u; i < outputCountInBatch; ++i) {
			const Batch::Output& output = batch.outputs[i];
			sharedContext.outputImages[i].save(output.path, output.format);
		}
	}

	// Clean extra compiled nodes
	for( NodeAndRegisters& compiledNode : compiledNodes ){
		if( compiledNode.node->type() > NodeClass::COUNT_EXPOSED ){
			delete compiledNode.node;
			compiledNode.node = nullptr;
		}
	}
	return true;
}

