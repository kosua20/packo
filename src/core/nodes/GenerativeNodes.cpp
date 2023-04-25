#include "core/nodes/GenerativeNodes.hpp"
#include "core/nodes/Nodes.hpp"
#include "core/Random.hpp"

ConstantFloatNode::ConstantFloatNode(){
	_name = "Constant";
	_description = "Constant scalar value.";
	_outputNames = {"X"};
	_attributes = { {"##X", Attribute::Type::FLOAT} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ConstantFloatNode, NodeClass::CONST_FLOAT, false, 1)

void ConstantFloatNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 1u);
	context.stack[outputs[0]] = _attributes[0].flt;
}

ConstantRGBANode::ConstantRGBANode(){
	_name = "Constant";
	_description = "Constant RGBA value.";
	_outputNames = { "R", "G", "B", "A" };
	_attributes = { {"##Val", Attribute::Type::COLOR} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ConstantRGBANode, NodeClass::CONST_COLOR, false, 1)

void ConstantRGBANode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 4u);
	for (uint i = 0u; i < 4u; ++i) {
		context.stack[outputs[i]] = _attributes[0].clr[i];
	}
}

UniformRandomNode::UniformRandomNode(){
	_name = "Random";
	_description = "Random value in [min, max[";
	_outputNames = { "X" };
	_attributes = { {"Min", Attribute::Type::FLOAT}, {"Max", Attribute::Type::FLOAT} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(UniformRandomNode, NodeClass::RANDOM_FLOAT, false, 1)

void UniformRandomNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 1u);
	const float mini = _attributes[0].flt;
	const float maxi = _attributes[1].flt;
	context.stack[outputs[0]] = Random::Float(mini, maxi);
}

RandomColorNode::RandomColorNode(){
	_name = "Random color";
	_description = "Random color in [0,1]^4";
	_outputNames = { "R", "G", "B", "A" };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(RandomColorNode, NodeClass::RANDOM_COLOR, false, 1)

void RandomColorNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 4u);
	const glm::vec4 rgba = Random::Color();
	for (uint i = 0u; i < 4u; ++i) {
		context.stack[outputs[i]] = rgba[i];
	}
}
