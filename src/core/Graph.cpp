#include "core/Graph.hpp"

void Graph::addNode(Node* node){
	uint index = _freeListNodes.getIndex();
	if(index == _nodes.size()){
		_nodes.push_back( node );
	} else {
		_nodes[index] = node;
	}
}

void Graph::removeNode(uint node){
	// Don't worry about links here.
	delete _nodes[node];
	_nodes[node] = nullptr;
	_freeListNodes.returnIndex(node);
}

void Graph::addLink(const Link& link){
	_links.push_back(link);
}

bool operator==(const Graph::Link& a, const Graph::Link& b){
	   return a.from.node == b.from.node && a.from.slot == b.from.slot
		   && a.to.node == b.to.node && a.to.slot == b.to.slot;
}

void GraphEditor::addNode(Node* node){
	if(std::find(_addedNodes.begin(), _addedNodes.end(), node) != _addedNodes.end()){
		return;
	}
	_addedNodes.push_back(node);
}

void GraphEditor::removeNode(uint node){
	assert(node < _graph._nodes.size());
	assert(_graph._nodes[node] != nullptr);

	_deletedNodes.insert(node);

	// Remove all associated links.
	const size_t linkCount = _graph.getLinkCount();
	for(size_t lid = 0; lid < linkCount; ++lid){
		const Graph::Link& link = _graph.link(lid);
		if(link.from.node == node || link.to.node == node){
			_deletedLinks.insert(lid);
		}
	}
	// For links that are waiting to be added, we'll handle them at the end.
}

void GraphEditor::addLink(uint fromNode, uint fromSlot, uint toNode, uint toSlot){
	Graph::Link link;
	link.from = {fromNode, fromSlot };
	link.to = { toNode, toSlot };

	if(std::find(_addedLinks.begin(), _addedLinks.end(), link) != _addedLinks.end()){
		return;
	}
	_addedLinks.push_back(link);
}

void GraphEditor::removeLink(uint link){
	assert(link < _graph._links.size());
	_deletedLinks.insert(link);
}

void GraphEditor::commit(){
	// Delete all nodes accumulated.
	for(uint nodeToDelete : _deletedNodes){
		_graph.removeNode(nodeToDelete);
	}

	// Removed added links that are not valid anymore.
	auto itl = std::remove_if(_addedLinks.begin(), _addedLinks.end(), [this](const Graph::Link& link){
		return _deletedNodes.count(link.from.node) > 0u || _deletedNodes.count(link.to.node) > 0u;
	});
	_addedLinks.erase(itl, _addedLinks.end());

	// Mark all links to delete with a sentinel node index.
	const uint kSentinel = 0xFFFFFFFF;
	assert(kSentinel >= _graph._nodes.size());
	for(uint linkToDelete : _deletedLinks){
		_graph._links[linkToDelete].from.node = kSentinel;
	}
	// Erase them.
	auto itg = std::remove_if(_graph._links.begin(), _graph._links.end(), [](const Graph::Link& link){
		return link.from.node == kSentinel;
	});
	_graph._links.erase(itg, _graph._links.end());

	// Add all new nodes
	for(Node* nodeToAdd : _addedNodes){
		_graph.addNode(nodeToAdd);
	}
	// and new links that are still valid
	for(const Graph::Link& link : _addedLinks){
		_graph.addLink(link);
	}

	// Done!
	_deletedNodes.clear();
	_deletedLinks.clear();
}
