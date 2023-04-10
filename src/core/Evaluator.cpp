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

	struct Child {
		const Vertex* node;
		std::vector<Edge> edges;

		Child(const Vertex* _node, uint from, uint to) : node(_node) {
			edges.push_back({from, to});
		}

		void addEdge(uint from, uint to){
			edges.push_back({from, to});
		}
	};

	struct Vertex {
		const Node* node;
		std::vector<Child> children;
		// TODO: parents?
		std::vector<uint> slots;

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

			std::vector<Child>& fromChildren = (*fromNode)->children;
			auto child = std::find_if(fromChildren.begin(), fromChildren.end(), [&toNode](const Child& child){
				return child.node == *toNode;
			});
			if(child == fromChildren.end()){
				fromChildren.emplace_back(*toNode, link.from.slot, link.to.slot);
			} else {
				child->addEdge( link.from.slot, link.to.slot);
			}
			(*toNode)->slots.push_back(link.to.slot);
		}
	}

	bool validateInputs(){
		bool incompleteNodes = false;
		// Check that all nodes have their inputs filled.
		for(uint nid = 0; nid < nodes.size();){
			const Vertex* node = nodes[nid];
			const uint tgtSlotCount = node->node->inputs().size();
			const auto& usedSlots = node->slots;
			bool missingSlot = false;
			for(uint sid = 0; sid < tgtSlotCount; ++sid){
				// Check if the slot is in use.
				if(std::find(usedSlots.begin(), usedSlots.end(), sid) != usedSlots.end()){
					continue;
				}
				_context.addError("Missing input", node->node, sid);
				missingSlot = true;
			}
			// If one slot is missing, remove the node from the graph.
			if(missingSlot){
				incompleteNodes = true;
			}
			++nid;
		}
		return !incompleteNodes;
	}

	bool hasCycle(const Vertex* node, std::vector<const Vertex*>& visited){
		// Have we already visited the node.
		if(std::find(visited.begin(), visited.end(), node) != visited.end()){
			_context.addError("Cycle detected", node->node);
			return true;
		}
		visited.push_back(node);
		bool childHasCycle = false;
		for(const Child& child : node->children){
			childHasCycle |= hasCycle(child.node, visited);
		}
		visited.pop_back();
		return childHasCycle;
	}

	bool validateCycles(){
		std::vector<const Vertex*> visited;
		visited.reserve(nodes.size());
		// Collect roots.
		std::vector<const Vertex*> roots;
		for(const Vertex* node : nodes){
			if(node->slots.empty()){
				roots.push_back(node);
			}
		}
		bool rootHasCycle = false;
		for(const Vertex* root : roots){
			rootHasCycle |= hasCycle(root, visited);
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

	// Validate the graph.
	WorkGraph graph(editGraph, errors);

	validate(graph);

	// Split the graph and order nodes to evaluate them.

	// For each node list all the flushing parent nodes, then greedily cluster them
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
	std::vector<Batch> batches(batchCount);

	for(uint batchId = 0u; batchId < batchCount; ++batchId){
		Batch& batch = batches[batchId];
		for(uint inputId = 0u; inputId < inputCount; ++inputId){
			const uint pathId = batchId * inputCount + inputId;
			if(pathId < pathCount){
				batch.inputs.push_back(inputPaths[pathId]);
			} else {
				// Empty input image will return 0s.
				// TODO: or an error?
				batch.inputs.push_back("");
			}
		}
		for(uint outputId = 0u; outputId < outputCount; ++outputId){
			const OutputNode* node = static_cast<const OutputNode*>(outputs[outputId]->node);
			const std::string outName = node->generateFileName(batchId);
			batch.outputs.push_back(outName);
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

