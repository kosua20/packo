#include "core/nodes/ArithmeticNodes.hpp"
#include "core/nodes/Nodes.hpp"

AddNode::AddNode(){

	_name = "Add";
	_inputNames = {"X", "Y"};
	_outputNames = {"X+Y"};
	
}

uint AddNode::type() const {
	return NodeClass::ADD;
}

uint AddNode::version() const{
	return 1;
}
