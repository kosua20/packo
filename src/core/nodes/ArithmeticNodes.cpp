#include "core/nodes/ArithmeticNodes.hpp"
#include "core/nodes/Nodes.hpp"

FreeList AddNode::_freeList;

AddNode::AddNode(){
	
	_index = _freeList.getIndex();
	_name = "Add " + std::to_string(_index);
	_inputNames = {"X", "Y"};
	_outputNames = {"X+Y"};
	
}

AddNode::~AddNode(){
	_freeList.returnIndex( _index );
}

uint AddNode::type() const {
	return NodeClass::ADD;
}

uint AddNode::version() const{
	return 1;
}
