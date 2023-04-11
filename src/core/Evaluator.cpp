#include "core/Evaluator.hpp"
#include "core/Graph.hpp"
#include "core/nodes/Nodes.hpp"

#include <unordered_map>
#include <sstream>

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
			if(node->node->type() != NodeClass::OUTPUT){
				continue;
			}
			// Generate the filename
			const OutputNode* outNode = static_cast<const OutputNode*>(node->node);
			const std::string filename = outNode->generateFileName(0);
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

	void collectInputsAndOutputs(std::vector<Vertex*>& inputs, std::vector<Vertex*>& outputs){
		for(Vertex* node : nodes){
			if(node->node->type() == NodeClass::INPUT){
				inputs.push_back(node);
			}
			if(node->node->type() == NodeClass::OUTPUT){
				outputs.push_back(node);
			}
		}
		auto sortByName = [](const Vertex* a, const Vertex* b){
			return a->node->name() < b->node->name();
		};

		std::sort(inputs.begin(), inputs.end(), sortByName);
		std::sort(outputs.begin(), outputs.end(), sortByName);
	}


	void checkConnectionToOutputs(Vertex* vertex){
		if(vertex->tmpData == 1u){
			return;
		}

		if(vertex->node->type() == NodeClass::OUTPUT){
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

	void orderNodes(){
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
		// Then assign registers
		purgeTmpData();
		uint id = 0;
		for(Vertex* vert : nodes){
			vert->tmpData = id;
			++id;
		}

		FreeList availableRegisters;
		struct Registers {
			std::vector<int> inputs;
			std::vector<int> outputs;
			std::vector<uint> outputsRefCount;
		};
		std::vector<Registers> assignedRegisters(nodes.size());
		for(Vertex* vert : nodes){
			Registers& registers = assignedRegisters[vert->tmpData];
			registers.inputs.resize(vert->node->inputs().size(), -1);
			registers.outputs.resize(vert->node->outputs().size(), -1);
			registers.outputsRefCount.resize(vert->node->outputs().size(), 0);
			for(Neighbor& child : vert->children){
				for(Edge& edge : child.edges){
					++registers.outputsRefCount[edge.from];
				}
			}
		}

		for(Vertex* vert : nodes){
			Registers& registers = assignedRegisters[vert->tmpData];
			for(Neighbor& parent : vert->parents){
				Registers& parentRegisters = assignedRegisters[parent.node->tmpData];
				for(Edge& edge : parent.edges){
					const int linkRegister = parentRegisters.outputs[edge.from];
					assert(linkRegister >= 0);
					registers.inputs[edge.to] = linkRegister;

					// We want to return registers from parents that have propagated them to all their children.
					--parentRegisters.outputsRefCount[edge.from];
					if(parentRegisters.outputsRefCount[edge.from] == 0u){
						availableRegisters.returnIndex(linkRegister);
					}
				}

			}
			for(Neighbor& child : vert->children){
				for(Edge& edge : child.edges){
					if(registers.outputs[edge.from] == -1){
						registers.outputs[edge.from] = availableRegisters.getIndex();
					}
				}
			}

		}

		for(const Vertex* vert : nodes){
			Log::Info() << "Processing node: " << vert->node->name() << " of type " << getNodeName(NodeClass(vert->node->type())) << "\n";
			const Registers& regs = assignedRegisters[vert->tmpData];
			Log::Info() << "Inputs: " << "\n";
			for(uint reg : regs.inputs){
				Log::Info() << reg << ", ";
			}
			Log::Info() << "\n";
			Log::Info() << "Outputs: " << "\n";
			for(uint reg : regs.outputs){
				Log::Info() << reg << ", ";
			}
			Log::Info() << std::endl;
		}
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


bool evaluate(const Graph& editGraph, ErrorContext& errors, const std::vector<std::string>& inputPaths, const std::string& outputDir ){

	errors.clear();

	// TODO: one base graph and N subgraphs.
	// Just create the raw graph and a subgraph with everything, validate, then split.
	// Validate the graph.
	WorkGraph graph(editGraph, errors);

	if(!validate(graph)){
		return false;
	}
	// We dont have incomplete nodes anymore.
	// We could have unconnected regions.
	graph.cleanUnconnectedComponents();

	// Split the graph and order nodes to evaluate them.
	// For each node list all the flushing parent nodes, then greedily cluster them
	graph.orderNodes();
	// For now assume a unique subgraph
	// Pick an input, queue all nodes that have no other dependencies.
	// Pick another input, idem.
	// Assign registers to each node

	// Create batches
	std::vector<WorkGraph::Vertex*> inputs;
	std::vector<WorkGraph::Vertex*> outputs;
	graph.collectInputsAndOutputs(inputs, outputs);

	// TODO: maps instead with pointers to the nodes?
	struct Batch {
		std::vector<std::string> inputs;
		std::vector<std::string> outputs;
	};

	const uint inputCount = inputs.size();
	const uint outputCount = outputs.size();
	const uint pathCount = inputPaths.size();
	const uint batchCount = pathCount / inputCount;

	// Check that we have the correct number of input paths
	if(batchCount * inputCount != pathCount){
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
			const OutputNode* node = static_cast<const OutputNode*>(outputs[outputId]->node);
			const std::string outName = node->generateFileName(batchId);
			batch.outputs.push_back(outputDir + "/" + outName);
		}
	}


	// Load all inputs, allocate all outputs, for the current batch
	for(const Batch& batch : batches){
		Log::Info() << "Processing batch: " << "\n";
		Log::Info() << "Inputs: " << "\n";
		for(const std::string& input : batch.inputs){
			Log::Info() << "* '" << input << "'" << "\n";
		}
		Log::Info() << "Outputs: " << "\n";
		for(const std::string& output : batch.outputs){
			Log::Info() << "* '" << output << "'" << "\n";
		}
	}
	// For each channel, evaluate the nodes using temporary storage.
	return true;
}

