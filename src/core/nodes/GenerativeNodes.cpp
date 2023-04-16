#include "core/nodes/GenerativeNodes.hpp"
#include "core/nodes/Nodes.hpp"
#include "core/Random.hpp"

ConstantFloatNode::ConstantFloatNode(){
	_name = "Constant";
	_outputNames = {"Value"};
	_attributes = { {"##X", Attribute::Type::FLOAT} };
}

NODE_DEFINE_TYPE_AND_VERSION(ConstantFloatNode, NodeClass::CONST_FLOAT, 1)

void ConstantFloatNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 1u);
	context.stack[outputs[0]] = _attributes[0].flt;
}

ConstantRGBANode::ConstantRGBANode(){
	_name = "Constant";
	_outputNames = { "R", "G", "B", "A" };
	_attributes = { {"##Val", Attribute::Type::COLOR} };
}

NODE_DEFINE_TYPE_AND_VERSION(ConstantRGBANode, NodeClass::CONST_COLOR, 1)

void ConstantRGBANode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 4u);
	for (uint i = 0u; i < 4u; ++i) {
		context.stack[outputs[i]] = _attributes[0].clr[i];
	}
}

UniformRandomNode::UniformRandomNode(){
	_name = "Random";
	_outputNames = { "X" };
	_attributes = { {"Min", Attribute::Type::FLOAT}, {"Max", Attribute::Type::FLOAT} };
}

NODE_DEFINE_TYPE_AND_VERSION(UniformRandomNode, NodeClass::RANDOM_FLOAT, 1)

void UniformRandomNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 1u);
	const float mini = _attributes[0].flt;
	const float maxi = _attributes[1].flt;
	context.stack[outputs[0]] = Random::Float(mini, maxi);
}

RandomColorNode::RandomColorNode(){
	_name = "Random color";
	_outputNames = { "R", "G", "B", "A" };
}

NODE_DEFINE_TYPE_AND_VERSION(RandomColorNode, NodeClass::RANDOM_COLOR, 1)

void RandomColorNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 4u);
	const glm::vec4 rgba = Random::Color();
	for (uint i = 0u; i < 4u; ++i) {
		context.stack[outputs[i]] = rgba[i];
	}
}
