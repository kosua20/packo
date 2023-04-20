#include "core/nodes/ArithmeticNodes.hpp"
#include "core/nodes/Nodes.hpp"

AddNode::AddNode(){
	_name = "Add";
	_inputNames = {"X", "Y"};
	_outputNames = {"X+Y"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(AddNode, NodeClass::ADD, true, 1)

void AddNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = context.stack[inputs[i]] + context.stack[inputs[i+_channelCount]];
	}
}


SubtractNode::SubtractNode(){
	_name = "Minus";
	_inputNames = {"X", "Y"};
	_outputNames = {"X-Y"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(SubtractNode, NodeClass::SUBTRACT, true, 1)

void SubtractNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = context.stack[inputs[i]] - context.stack[inputs[i+_channelCount]];
	}
}

ProductNode::ProductNode(){
	_name = "Product";
	_inputNames = {"X", "Y"};
	_outputNames = {"X*Y"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ProductNode, NodeClass::PRODUCT, true, 1)

void ProductNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = context.stack[inputs[i]] * context.stack[inputs[i+_channelCount]];
	}
}

DivideNode::DivideNode(){
	_name = "Divide";
	_inputNames = {"X", "Y"};
	_outputNames = {"X/Y"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(DivideNode, NodeClass::DIVIDE, true, 1)

void DivideNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = context.stack[inputs[i]] / context.stack[inputs[i+_channelCount]];
	}
}

MinNode::MinNode(){
	_name = "Minimum";
	_inputNames = {"X", "Y"};
	_outputNames = {"min(X,Y)"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(MinNode, NodeClass::MINI, true, 1)

void MinNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::min(context.stack[inputs[i]], context.stack[inputs[i+_channelCount]]);
	}
}

MaxNode::MaxNode(){
	_name = "Maximum";
	_inputNames = {"X", "Y"};
	_outputNames = {"max(X,Y)"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(MaxNode, NodeClass::MAXI, true, 1)

void MaxNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::max(context.stack[inputs[i]], context.stack[inputs[i+_channelCount]]);
	}
}

ClampNode::ClampNode(){
	_name = "Clamp";
	_inputNames = {"X"};
	_outputNames = {"max(min(X,A),B)"};
	_attributes = { {"A", Attribute::Type::FLOAT}, {"B", Attribute::Type::FLOAT} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ClampNode, NodeClass::CLAMP, true, 1)

void ClampNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::clamp(context.stack[inputs[i]], _attributes[0].flt, _attributes[1].flt);
	}
}

PowerNode::PowerNode(){
	_name = "Power";
	_inputNames = {"X", "Y"};
	_outputNames = {"X^Y"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(PowerNode, NodeClass::POWER, true, 1)

void PowerNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::pow(context.stack[inputs[i]], context.stack[inputs[i+_channelCount]]);
	}
}

SqrtNode::SqrtNode(){
	_name = "Square root";
	_inputNames = {"X"};
	_outputNames = {"sqrt(X)"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(SqrtNode, NodeClass::SQRT, true, 1)

void SqrtNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::sqrt(context.stack[inputs[i]]);
	}
}

ExponentialNode::ExponentialNode(){
	_name = "Exponential";
	_inputNames = {"X"};
	_outputNames = {"exp(X)"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ExponentialNode, NodeClass::EXPONENTIAL, true, 1)

void ExponentialNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::exp(context.stack[inputs[i]]);
	}
}

LogarithmNode::LogarithmNode(){
	_name = "Logarithm";
	_inputNames = {"X"};
	_outputNames = {"log(X)"};
	_attributes = {{"Basis", Attribute::Type::FLOAT}};
	_attributes[0].flt = glm::e<float>();
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(LogarithmNode, NodeClass::LOGARITHM, true, 1)

void LogarithmNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::log(context.stack[inputs[i]]) / glm::log(_attributes[0].flt);
	}
}

MixNode::MixNode(){
	_name = "Interpolate";
	_inputNames = {"X", "Y", "T"};
	_outputNames = {"mix(X,Y,T)"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(MixNode, NodeClass::MIX, true, 1)

void MixNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 3 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::mix(context.stack[inputs[i]], context.stack[inputs[i+_channelCount]], context.stack[inputs[i+2*_channelCount]]);
	}
}
