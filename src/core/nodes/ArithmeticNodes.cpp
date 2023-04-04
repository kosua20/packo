#include "core/nodes/ArithmeticNodes.hpp"


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
