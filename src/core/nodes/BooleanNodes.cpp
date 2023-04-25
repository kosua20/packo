#include "core/nodes/ArithmeticNodes.hpp"
#include "core/nodes/Nodes.hpp"

SelectNode::SelectNode(){
	_name = "Select";
	_description = "M = if B then X else Y";
	_inputNames = {"X", "Y", "B"};
	_outputNames = {"M"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(SelectNode, NodeClass::SELECT, true, 1)

void SelectNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 3 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = context.stack[inputs[i+2*_channelCount]] > 0.5f ? context.stack[inputs[i]] : context.stack[inputs[i+_channelCount]];
	}
}

static constexpr float kEpsilon = 1e-5f;

EqualNode::EqualNode(){
	_name = "Equal";
	_description = "B = (X==Y) ?";
	_inputNames = {"X", "Y"};
	_outputNames = {"B"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(EqualNode, NodeClass::EQUAL, true, 1)

void EqualNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		const float dist = std::abs(context.stack[inputs[i]] - context.stack[inputs[i+_channelCount]]);
		context.stack[outputs[i]] = float(dist < kEpsilon);
	}
}

DifferentNode::DifferentNode(){
	_name = "Different";
	_description = "B = (X!=Y) ?";
	_inputNames = {"X", "Y"};
	_outputNames = {"B"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(DifferentNode, NodeClass::DIFFERENT, true, 1)

void DifferentNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		const float dist = std::abs(context.stack[inputs[i]] - context.stack[inputs[i+_channelCount]]);
		context.stack[outputs[i]] = float(dist >= kEpsilon);
	}
}

GreaterNode::GreaterNode(){
	_name = "Greater";
	_description = "B = X>Y (strict)\nB = X≥Y (otherwise)";
	_inputNames = {"X", "Y"};
	_outputNames = {"B"};
	_attributes = {{"Strict", Attribute::Type::BOOL}};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(GreaterNode, NodeClass::GREATER, true, 1)

void GreaterNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);

	if(_attributes[0].bln){
		for(uint i = 0; i < _channelCount; ++i){
			context.stack[outputs[i]] = float(context.stack[inputs[i]] > context.stack[inputs[i+_channelCount]]);
		}
	} else {
		for(uint i = 0; i < _channelCount; ++i){
			context.stack[outputs[i]] = float(context.stack[inputs[i]] >= context.stack[inputs[i+_channelCount]]);
		}
	}

}

LessNode::LessNode(){
	_name = "Less";
	_description = "B = X<Y (strict)\nB = X≤Y (otherwise)";
	_inputNames = {"X", "Y"};
	_outputNames = {"B"};
	_attributes = {{"Strict", Attribute::Type::BOOL}};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(LessNode, NodeClass::LESSER, true, 1)

void LessNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	if(_attributes[0].bln){
		for(uint i = 0; i < _channelCount; ++i){
			context.stack[outputs[i]] = float(context.stack[inputs[i]] < context.stack[inputs[i+_channelCount]]);
		}
	} else {
		for(uint i = 0; i < _channelCount; ++i){
			context.stack[outputs[i]] = float(context.stack[inputs[i]] <= context.stack[inputs[i+_channelCount]]);
		}
	}
}

NegateNode::NegateNode(){
	_name = "Not";
	_description = "B= not X";
	_inputNames = {"X"};
	_outputNames = {"B"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(NegateNode, NodeClass::NOT, true, 1)

void NegateNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = float(context.stack[inputs[i]] <= 0.5f);
	}
}

