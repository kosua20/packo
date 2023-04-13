#include "core/nodes/ArithmeticNodes.hpp"
#include "core/nodes/Nodes.hpp"

AddNode::AddNode(){
	_name = "Add";
	_inputNames = {"X", "Y"};
	_outputNames = {"X+Y"};
}

NODE_DEFINE_TYPE_AND_VERSION(AddNode, NodeClass::ADD, 1)

void AddNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = context.stack[inputs[0]] + context.stack[inputs[1]];
}


SubtractNode::SubtractNode(){
	_name = "Minus";
	_inputNames = {"X", "Y"};
	_outputNames = {"X-Y"};
}

NODE_DEFINE_TYPE_AND_VERSION(SubtractNode, NodeClass::SUBTRACT, 1)

void SubtractNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = context.stack[inputs[0]] - context.stack[inputs[1]];
}

ProductNode::ProductNode(){
	_name = "Product";
	_inputNames = {"X", "Y"};
	_outputNames = {"X*Y"};
}

NODE_DEFINE_TYPE_AND_VERSION(ProductNode, NodeClass::PRODUCT, 1)

void ProductNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = context.stack[inputs[0]] * context.stack[inputs[1]];
}

DivideNode::DivideNode(){
	_name = "Divide";
	_inputNames = {"X", "Y"};
	_outputNames = {"X/Y"};
}

NODE_DEFINE_TYPE_AND_VERSION(DivideNode, NodeClass::DIVIDE, 1)

void DivideNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = context.stack[inputs[0]] / context.stack[inputs[1]];
}

MinNode::MinNode(){
	_name = "Minimum";
	_inputNames = {"X", "Y"};
	_outputNames = {"min(X,Y)"};
}

NODE_DEFINE_TYPE_AND_VERSION(MinNode, NodeClass::MINI, 1)

void MinNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = glm::min(context.stack[inputs[0]], context.stack[inputs[1]]);
}

MaxNode::MaxNode(){
	_name = "Maximum";
	_inputNames = {"X", "Y"};
	_outputNames = {"max(X,Y)"};
}

NODE_DEFINE_TYPE_AND_VERSION(MaxNode, NodeClass::MAXI, 1)

void MaxNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = glm::max(context.stack[inputs[0]], context.stack[inputs[1]]);
}

ClampNode::ClampNode(){
	_name = "Clamp";
	_inputNames = {"X"};
	_outputNames = {"max(min(X,A),B)"};
	_attributes = { {"A", Attribute::Type::FLOAT}, {"B", Attribute::Type::FLOAT} };
}

NODE_DEFINE_TYPE_AND_VERSION(ClampNode, NodeClass::CLAMP, 1)

void ClampNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = glm::clamp(context.stack[inputs[0]], _attributes[0].flt, _attributes[1].flt);
}

PowerNode::PowerNode(){
	_name = "Power";
	_inputNames = {"X", "Y"};
	_outputNames = {"X^Y"};
}

NODE_DEFINE_TYPE_AND_VERSION(PowerNode, NodeClass::POWER, 1)

void PowerNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = glm::pow(context.stack[inputs[0]], context.stack[inputs[1]]);
}

SqrtNode::SqrtNode(){
	_name = "Square root";
	_inputNames = {"X"};
	_outputNames = {"sqrt(X)"};
}

NODE_DEFINE_TYPE_AND_VERSION(SqrtNode, NodeClass::SQRT, 1)

void SqrtNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = glm::sqrt(context.stack[inputs[0]]);
}

ExponentialNode::ExponentialNode(){
	_name = "Exponential";
	_inputNames = {"X"};
	_outputNames = {"exp(X)"};
}

NODE_DEFINE_TYPE_AND_VERSION(ExponentialNode, NodeClass::EXPONENTIAL, 1)

void ExponentialNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = glm::exp(context.stack[inputs[0]]);
}

LogarithmNode::LogarithmNode(){
	_name = "Logarithm";
	_inputNames = {"X"};
	_outputNames = {"log(X)"};
	_attributes = {{"Basis", Attribute::Type::FLOAT}};
	_attributes[0].flt = glm::e<float>();
}

NODE_DEFINE_TYPE_AND_VERSION(LogarithmNode, NodeClass::LOGARITHM, 1)

void LogarithmNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = glm::log(context.stack[inputs[0]]) / glm::log(_attributes[0].flt);
}
