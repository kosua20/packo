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

GradientNode::GradientNode(){
	_name = "Gradient";
	_description = "Gradient in X, Y directions, radial and angular";
	_outputNames = { "X", "Y", "R", "T" };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(GradientNode, NodeClass::GRADIENT, false, 1)

void GradientNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 4u);

	const glm::vec2 uv = (glm::vec2(context.coords) + 0.5f) / glm::vec2(context.shared->dims);
	const glm::vec2 ndc = 2.f * uv - 1.0f;
	const float radius = glm::min(glm::length(ndc), 1.f);
	float angle = ndc.x == 0.f ? glm::sign(ndc.y) * glm::half_pi<float>() : glm::atan(ndc.y, ndc.x);
	angle = angle / glm::pi<float>() * 0.5f + 0.5f;

	const glm::vec4 result(uv.x, uv.y, radius, angle);

	for (uint i = 0u; i < 4u; ++i) {
		context.stack[outputs[i]] = result[i];
	}
}
