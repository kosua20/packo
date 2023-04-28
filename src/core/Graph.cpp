#include "core/Graph.hpp"
#include <json/json.hpp>
#include "core/nodes/Nodes.hpp"

Graph::~Graph(){
	clear();
}

int Graph::findNode(const Node* const node){
	if(node == nullptr)
		return -1;

	for(uint nodeIndex = 0u; nodeIndex < _nodes.size(); ++nodeIndex){
		if(_nodes[ nodeIndex ] == node)
			return (int)nodeIndex;
	}
	return -1;
}

int Graph::findLink( const Link& link ){
	for( uint linkIndex = 0u; linkIndex < _links.size(); ++linkIndex ){
		if( _links[ linkIndex ] == link )
			return (int)linkIndex;
	}
	return -1;
}

uint Graph::addNode(Node* node){
	uint index = _freeListNodes.getIndex();
	if(index == _nodes.size()){
		_nodes.push_back( node );
	} else {
		_nodes[index] = node;
	}
	return index;
}

void Graph::removeNode(uint node){
	if(_nodes[node] == nullptr){
		return;
	}
	// Don't worry about links here.
	delete _nodes[node];
	_nodes[node] = nullptr;
	_freeListNodes.returnIndex(node);
}

void Graph::addLink(const Link& link){
	_links.push_back(link);
}

void Graph::serialize(json& data){
	data["nodes"] = {};
	data["links"] = {};
	for(const Node* node : _nodes){
		auto& nodeData = data["nodes"].emplace_back();
		if(node == nullptr)
			continue;
		node->serialize(nodeData);
	}
	for(const Link& link : _links ){
		auto& linkData = data["links"].emplace_back();
		linkData["from"]["node"] = link.from.node;
		linkData["from"]["slot"] = link.from.slot;
		linkData["to"]["node"] = link.to.node;
		linkData["to"]["slot"] = link.to.slot;
	}
}

bool Graph::deserialize(const json& data){
	clear();

	if(data.contains("nodes")){

		for(auto& nodeData : data["nodes"]){
			// Ensure the free list is in the correct state.
			uint nodeId =_freeListNodes.getIndex();
			(void)nodeId;
			if(nodeData.is_null() || !nodeData.contains("type")){
				_nodes.push_back(nullptr);
			} else {
				uint type = nodeData["type"];
				type = (std::min)(uint(NodeClass::COUNT_EXPOSED), type);
				Node* node = createNode(NodeClass(type));
				if(node){
					if(!node->deserialize(nodeData)){
						delete node;
						node = nullptr;
					}
				}
				_nodes.push_back(node);
			}
		}

		const uint nodeCount = _nodes.size();
		for(uint nodeId = 0u; nodeId < nodeCount; ++nodeId){
			if(!_nodes[nodeId]){
				_freeListNodes.returnIndex(nodeId);
			}
		}
	}
	if(data.contains("links")){
		for(auto& linkData : data["links"]){
			if(!linkData.contains("from") || !linkData.contains("to")){
				continue;
			}
			const auto& fromData = linkData["from"];
			if(!fromData.contains("node") || !fromData.contains("slot")){
				continue;
			}
			const auto& toData = linkData["to"];
			if(!toData.contains("node") || !toData.contains("slot")){
				continue;
			}
			Link& link = _links.emplace_back();
			link.from.node = fromData["node"];
			link.from.slot = fromData["slot"];
			link.to.node = toData["node"];
			link.to.slot = toData["slot"];
		}
	}
	return true;
}

void Graph::clear(){
	uint nodesCount = _nodes.size();
	for(uint nodeId = 0u; nodeId < nodesCount; ++nodeId){
		removeNode(nodeId);
	}
	_nodes.clear();
	_links.clear();
}

bool operator==(const Graph::Link& a, const Graph::Link& b){
	   return a.from.node == b.from.node && a.from.slot == b.from.slot
		   && a.to.node == b.to.node && a.to.slot == b.to.slot;
}

GraphEditor::GraphEditor( Graph& graph ) : _graph( graph ) { 
	// Ensure we avoid collision with existing node indices.
	_firstFutureNodeId = graph.getNodeCountUpperBound();
	_nextFutureNodeId = _firstFutureNodeId;
}

GraphEditor::~GraphEditor(){
	// Make sure all modifications are complete.
	commit();
}

uint GraphEditor::addNode(Node* node){
	auto existingNode = std::find_if( _addedNodes.begin(), _addedNodes.end(), [&node] ( const FutureNode& a ){
		return a.node == node;
	});
	if( existingNode != _addedNodes.end()){
		return existingNode->id;
	}
	const uint futureId = _nextFutureNodeId;
	++_nextFutureNodeId;
	_addedNodes.push_back( { node, futureId } );
	return futureId;
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

	// Remove previously added links pointing to the same destination (including copies of this one).
	auto itl = std::remove_if( _addedLinks.begin(), _addedLinks.end(), [&link] ( const Graph::Link& olink )
	{
		return olink.to.node == link.to.node && olink.to.slot == link.to.slot;
	});
	_addedLinks.erase( itl, _addedLinks.end() );

	// Remove existing links pointing to the same destination.
	const size_t linkCount = _graph.getLinkCount();
	for( size_t lid = 0; lid < linkCount; ++lid )
	{
		const Graph::Link& olink = _graph.link( lid );
		if( olink.to.node == link.to.node && olink.to.slot == link.to.slot )
		{
			_deletedLinks.insert( lid );
		}
	}

	// Add new link
	_addedLinks.push_back( link );
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
	constexpr uint kSentinel = 0xFFFFFFFF;

	assert(kSentinel >= _graph._nodes.size());
	for(uint linkToDelete : _deletedLinks){
		_graph._links[linkToDelete].from.node = kSentinel;
	}
	// Erase them.
	auto itg = std::remove_if(_graph._links.begin(), _graph._links.end(), [kSentinel](const Graph::Link& link){
		return link.from.node == kSentinel;
	});
	_graph._links.erase(itg, _graph._links.end());

	// Add all new nodes
	std::unordered_map<uint, uint> futureToRealIndices;
	for(auto& nodeToAdd : _addedNodes){
		// Update with the real index.
		uint realId = _graph.addNode(nodeToAdd.node);
		futureToRealIndices[ nodeToAdd.id ] = realId;
	}

	// and new links that are still valid
	for(const Graph::Link& link : _addedLinks){
		// TODO: check in table above if an index is a "future", in which case replace it by the real one.
		_graph.addLink(link);
	}

	// Done!
	_deletedNodes.clear();
	_deletedLinks.clear();
	_addedLinks.clear();
	_addedNodes.clear();
	_firstFutureNodeId = _graph.getNodeCountUpperBound();
	_nextFutureNodeId = _firstFutureNodeId;
}
