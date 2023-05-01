#include "core/nodes/ArithmeticNodes.hpp"
#include "core/nodes/Nodes.hpp"

AddNode::AddNode(){
	_name = "Add";
	_description = "M=X+Y";
	_inputNames = {"X", "Y"};
	_outputNames = {"M"};
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
	_description = "M=X-Y";
	_inputNames = {"X", "Y"};
	_outputNames = {"M"};
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
	_description = "M=X*Y";
	_inputNames = {"X", "Y"};
	_outputNames = {"M"};
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
	_description = "M=X/Y";
	_inputNames = {"X", "Y"};
	_outputNames = {"M"};
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
	_description = "M=min(X,Y)";
	_inputNames = {"X", "Y"};
	_outputNames = {"M"};
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
	_description = "M=max(X,Y)";
	_inputNames = {"X", "Y"};
	_outputNames = {"M"};
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
	_description = "M=min(max(X,A),B)";
	_inputNames = {"X"};
	_outputNames = { "M" };
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
	_description = "M=X^Y";
	_inputNames = {"X", "Y"};
	_outputNames = {"M"};
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
	_description = "M=sqrt(X)";
	_inputNames = {"X"};
	_outputNames = {"M"};
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
	_description = "M=exp(X)";
	_inputNames = {"X"};
	_outputNames = {"M"};
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
	_description = "M=log_basis(X)=ln(X)/ln(basis)";
	_inputNames = {"X"};
	_outputNames = {"M"};
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
	_description = "M=mix(X,Y,T)=(1-T)*X+T*Y";
	_inputNames = {"X", "Y", "T"};
	_outputNames = {"M"};
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

SinNode::SinNode(){
	_name = "Sine";
	_description = "M=sin(X)";
	_inputNames = {"X"};
	_outputNames = {"M"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(SinNode, NodeClass::SINE, true, 1)

void SinNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::sin(context.stack[inputs[i]]);
	}
}

CosNode::CosNode(){
	_name = "Cosine";
	_description = "M=cos(X)";
	_inputNames = {"X"};
	_outputNames = {"M"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(CosNode, NodeClass::COSINE, true, 1)

void CosNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::cos(context.stack[inputs[i]]);
	}
}

TanNode::TanNode(){
	_name = "Tangent";
	_description = "M=tan(X)";
	_inputNames = {"X"};
	_outputNames = {"M"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(TanNode, NodeClass::TANGENT, true, 1)

void TanNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::tan(context.stack[inputs[i]]);
	}
}

ArcSinNode::ArcSinNode(){
	_name = "Arc Sine";
	_description = "M=asin(X)";
	_inputNames = {"X"};
	_outputNames = {"M"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ArcSinNode, NodeClass::ARCSINE, true, 1)

void ArcSinNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::asin(context.stack[inputs[i]]);
	}
}

ArcCosNode::ArcCosNode(){
	_name = "Arc Cosine";
	_description = "M=acos(X)";
	_inputNames = {"X"};
	_outputNames = {"M"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ArcCosNode, NodeClass::ARCCOSINE, true, 1)

void ArcCosNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::acos(context.stack[inputs[i]]);
	}
}

ArcTanNode::ArcTanNode(){
	_name = "Tangent";
	_description = "M=atan(X)";
	_inputNames = {"X"};
	_outputNames = {"M"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ArcTanNode, NodeClass::ARCTANGENT, true, 1)

void ArcTanNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::atan(context.stack[inputs[i]]);
	}
}

DotProductNode::DotProductNode(){
	_name = "Dot product";
	_description = "M=dot(X,Y)";
	_inputNames = {"X", "Y"};
	_outputNames = {"M"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(DotProductNode, NodeClass::DOT, true, 1)

void DotProductNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);

	float dot = 0.f;
	for(uint i = 0; i < _channelCount; ++i){
		const float y = context.stack[inputs[i + _channelCount]];
		const float x = context.stack[inputs[i]];
		dot += x * y;
	}
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = dot;
	}
}

AbsNode::AbsNode(){
	_name = "Absolute value";
	_description = "M=|X|";
	_inputNames = {"X"};
	_outputNames = {"M"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(AbsNode, NodeClass::ABS, true, 1)

void AbsNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = std::abs(context.stack[inputs[i]]);
	}
}
