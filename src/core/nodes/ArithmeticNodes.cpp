#include "core/nodes/ArithmeticNodes.hpp"

AddNode::AddNode(){
	_name = "Add";
	_inputNames = {"X", "Y"};
	_outputNames = {"X+Y"};
}
