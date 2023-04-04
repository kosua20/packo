#include "core/Evaluator.hpp"
#include "core/Graph.hpp"
#include "core/nodes/Nodes.hpp"

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

	WorkGraph(const Graph& graph, ErrorContext& errorContext) : context(errorContext){

		GraphNodes nodesIt(graph);
		// Allocate nodes.
		for(uint node : nodesIt){
			nodesPool.emplace_back(graph.node(node));
		}
		// Insert working nodes.
		for(Vertex& node : nodesPool){
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
		bool missing = false;
		// Check that all nodes have their inputs filled.
		for(uint nid = 0; nid < nodes.size();){
			const Vertex* node = nodes[nid];
			if(node->slots.size() != node->node->inputCount()){
				// Missing input
				nodes[nid] = nodes.back();
				nodes.resize(nodes.size()-1);
				context.addError("Missing inputs", node->node);
				missing = true;
				continue;
			}
			++nid;
		}
		return !missing;
	}

	bool hasCycle(const Vertex* node, std::vector<const Vertex*>& visited){
		// Have we already visited the node.
		if(std::find(visited.begin(), visited.end(), node) != visited.end()){
			context.addError("Cycle detected", node->node);
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

	std::vector<Vertex*> nodes;

private:

	ErrorContext& context;

	std::vector<Vertex> nodesPool;

};


bool evaluate(const Graph& _editGraph, ErrorContext& _errors, const std::vector<std::string>& inputPaths, const std::string& outputDir ){

	_errors.clear();

	// Validate the graph.
	WorkGraph graph(_editGraph, _errors);

	if(!graph.validateInputs()){
		return false;
	}
	if(!graph.validateCycles()){
		return false;
	}
	// Split the graph and order nodes to evaluate them.
	// For each node list all the flushing parent nodes, then greedily cluster them
	// Assign registers to each node

	// Load all inputs, allocate all outputs, for the current batch
	// For each channel, evaluate the nodes using temporary storage.
	return true;
}

