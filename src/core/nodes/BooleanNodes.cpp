#include "core/nodes/ArithmeticNodes.hpp"
#include "core/nodes/Nodes.hpp"

SelectNode::SelectNode(){
	_name = "Select";
	_inputNames = {"X", "Y", "B"};
	_outputNames = {"B ? X else Y "};
}

NODE_DEFINE_TYPE_AND_VERSION(SelectNode, NodeClass::SELECT, 1)

void SelectNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 3);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = context.stack[inputs[2]] > 0.5f ? context.stack[inputs[0]] : context.stack[inputs[1]];
}

static constexpr float kEpsilon = 1e-5f;

EqualNode::EqualNode(){
	_name = "Equal";
	_inputNames = {"X", "Y"};
	_outputNames = {"X==Y"};
}

NODE_DEFINE_TYPE_AND_VERSION(EqualNode, NodeClass::EQUAL, 1)

void EqualNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2);
	assert(outputs.size() == 1);
	const float dist = std::abs(context.stack[inputs[0]] - context.stack[inputs[1]]);
	context.stack[outputs[0]] = float(dist < kEpsilon);
}

DifferentNode::DifferentNode(){
	_name = "Different";
	_inputNames = {"X", "Y"};
	_outputNames = {"X!=Y"};
}

NODE_DEFINE_TYPE_AND_VERSION(DifferentNode, NodeClass::DIFFERENT, 1)

void DifferentNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2);
	assert(outputs.size() == 1);
	const float dist = std::abs(context.stack[inputs[0]] - context.stack[inputs[1]]);
	context.stack[outputs[0]] = float(dist >= kEpsilon);
}

GreaterNode::GreaterNode(){
	_name = "Greater";
	_inputNames = {"X", "Y"};
	_outputNames = {"X>Y"};
	_attributes = {{"Strict", Attribute::Type::BOOL}};
}

NODE_DEFINE_TYPE_AND_VERSION(GreaterNode, NodeClass::GREATER, 1)

void GreaterNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2);
	assert(outputs.size() == 1);

	if(_attributes[0].bln){
		context.stack[outputs[0]] = float(context.stack[inputs[0]] > context.stack[inputs[1]]);
	} else {
		context.stack[outputs[0]] = float(context.stack[inputs[0]] >= context.stack[inputs[1]]);
	}

}

LessNode::LessNode(){
	_name = "Less";
	_inputNames = {"X", "Y"};
	_outputNames = {"X<Y"};
	_attributes = {{"Strict", Attribute::Type::BOOL}};
}

NODE_DEFINE_TYPE_AND_VERSION(LessNode, NodeClass::LESSER, 1)

void LessNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2);
	assert(outputs.size() == 1);
	if(_attributes[0].bln){
		context.stack[outputs[0]] = float(context.stack[inputs[0]] < context.stack[inputs[1]]);
	} else {
		context.stack[outputs[0]] = float(context.stack[inputs[0]] <= context.stack[inputs[1]]);
	}
}

NegateNode::NegateNode(){
	_name = "Not";
	_inputNames = {"X"};
	_outputNames = {"!X"};
}

NODE_DEFINE_TYPE_AND_VERSION(NegateNode, NodeClass::NOT, 1)

void NegateNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1);
	assert(outputs.size() == 1);
	context.stack[outputs[0]] = float(context.stack[inputs[0]] <= 0.5f);
}

