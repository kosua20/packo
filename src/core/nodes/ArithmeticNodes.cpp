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

void AddNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = context.stack[inputs[0]] + context.stack[inputs[1]];
}