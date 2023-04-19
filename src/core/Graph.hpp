#pragma once
#include "core/Common.hpp"
#include "core/FreeList.hpp"
#include "core/nodes/Node.hpp"
#include <set>

struct GraphNodes;
class GraphEditor;

class Graph {

	friend GraphNodes;
	friend GraphEditor;

public:

	struct Slot {
		uint node;
		uint slot;
	};

	struct Link {
		Slot from;
		Slot to;
	};

	~Graph();

	int findNode( const Node* node );
	int findLink( const Link& link );

	const Node* node( uint node ) const { return _nodes[ node ]; }

	Node* node(uint node) { return _nodes[node]; }

	uint getLinkCount() const { return _links.size(); }

	const Link& link(uint link) const { return _links[link]; }

	void serialize(json& data);
	bool deserialize(const json& data);

private:

	void addNode(Node* node);

	void removeNode(uint node);

	void addLink(const Graph::Link& link);

	void clear();

	std::vector<Node*> _nodes;
	std::vector<Link> _links;
	FreeList _freeListNodes;

};

bool operator==(const Graph::Link& a, const Graph::Link& b);

class GraphEditor {
public:

	GraphEditor(Graph& graph) : _graph(graph){}

	~GraphEditor();

	void addNode(Node* node);

	void addLink(uint fromNode, uint fromSlot, uint toNode, uint toSlot);

	void removeNode(uint node);

	void removeLink(uint link);

	void commit();

private:

	Graph& _graph;

	std::set<uint> _deletedNodes;
	std::set<uint> _deletedLinks;
	std::vector<Node*> _addedNodes;
	std::vector<Graph::Link> _addedLinks;

};


struct GraphNodes {
public:
	GraphNodes(const Graph& graph) : _graph(graph){}

	class iterator {
	public:
		explicit iterator(const Graph& graph, uint index) : _graph(graph), _index(index) {
			// Find the next valid index
			while(_index < _graph._nodes.size() && _graph._nodes[_index] == nullptr){
				++_index;
			}
		};

		iterator& operator++(){
			do {
				++_index;
			} while(_index < _graph._nodes.size() && _graph._nodes[_index] == nullptr);
			return *this;
		}

		iterator operator++(int){ iterator res = *this; ++(*this); return res;}

		bool operator==(iterator other) const { return _index == other._index; }
		bool operator!=(iterator other) const { return !(*this == other); }
		uint operator*(){ return _index; }

		using difference_type = std::ptrdiff_t;
		using value_type = uint;
		using pointer = const uint*;
		using reference = const uint&;
		using iterator_category = std::forward_iterator_tag;

	private:
		const Graph& _graph;
		uint _index{ 0 };
	};

	iterator begin() { return iterator(_graph, 0); }
	iterator end() { return iterator(_graph, _graph._nodes.size()); }

private:

	const Graph& _graph;

};


