#include "core/nodes/GenerativeNodes.hpp"
#include "core/nodes/Nodes.hpp"
#include "core/Random.hpp"

ConstantFloatNode::ConstantFloatNode(){
	_name = "Constant";
	_description = "Constant scalar value.";
	_outputNames = { {"X", true} };
	_attributes = { {"##X", Attribute::Type::FLOAT} };
	_attributes[0].flt = 1.0f;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ConstantFloatNode, NodeClass::CONST_FLOAT,  1)

void ConstantFloatNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 1u * _channelCount);
	(void)inputs;
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = _attributes[0].flt;
	}
}

ConstantRGBANode::ConstantRGBANode(){
	_name = "Constant";
	_description = "Constant RGBA value.";
	_outputNames = { {"R", false }, {"G", false }, {"B", false}, {"A", false} };
	_attributes = { {"##Val", Attribute::Type::COLOR} };
	_attributes[0].clr = glm::vec4(1.0f);
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ConstantRGBANode, NodeClass::CONST_COLOR, 1)

void ConstantRGBANode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 4u);
	(void)inputs;
	for (uint i = 0u; i < 4u; ++i) {
		context.stack[outputs[i]] = _attributes[0].clr[i];
	}
}

UniformRandomNode::UniformRandomNode(){
	_name = "Random";
	_description = "Random value in [min, max[";
	_outputNames = { {"X", true} };
	_attributes = { {"Min", Attribute::Type::FLOAT}, {"Max", Attribute::Type::FLOAT} };
	_attributes[0].flt = 0.f;
	_attributes[1].flt = 1.f;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(UniformRandomNode, NodeClass::RANDOM_FLOAT, 1)

void UniformRandomNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 1u * _channelCount);
	(void)inputs;
	const float mini = _attributes[0].flt;
	const float maxi = _attributes[1].flt;
	float val = Random::Float(mini, maxi);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = val;
	}
}

RandomColorNode::RandomColorNode(){
	_name = "Random color";
	_description = "Random color in [0,1]^4";
	_outputNames = { {"R", false }, {"G", false }, {"B", false}, {"A", false} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(RandomColorNode, NodeClass::RANDOM_COLOR, 1)

void RandomColorNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 4u);
	(void)inputs;

	const glm::vec4 rgba = Random::Color();
	for (uint i = 0u; i < 4u; ++i) {
		context.stack[outputs[i]] = rgba[i];
	}
}

GradientNode::GradientNode(){
	_name = "Gradient";
	_description = "Gradient: radial, angular, diamond, mirror";
	_outputNames = { {"R", false }, {"A", false }, {"D", false}, {"M", false} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(GradientNode, NodeClass::GRADIENT, 1)

void GradientNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 4u);
	(void)inputs;
	
	const glm::vec2 uv = (glm::vec2(context.coords) + 0.5f) / glm::vec2(context.shared->dims);
	const glm::vec2 ndc = 2.f * uv - 1.0f;
	const float radius = glm::min(glm::length(ndc), 1.f);
	float angle = ndc.x == 0.f ? glm::sign(ndc.y) * glm::half_pi<float>() : glm::atan(ndc.y, ndc.x);
	angle = angle / glm::pi<float>() * 0.5f + 0.5f;
		
	const float diamond = 1.f - (1.f - std::abs( ndc.x )) * (1.f - std::abs( ndc.y ));
	const float mirror = std::abs(ndc.x);
	const glm::vec4 result(radius, angle, diamond, mirror );

	for (uint i = 0u; i < 4u; ++i) {
		context.stack[outputs[i]] = result[i];
	}
}
