#include "core/nodes/ArithmeticNodes.hpp"
#include "core/nodes/Nodes.hpp"

AddNode::AddNode(){
	_name = "Add";
	_description = "M=X+Y";
	_inputNames = { {"X", true}, {"Y", true} };
	_outputNames = {{"M", true}};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(AddNode, NodeClass::ADD, 1)

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
	_inputNames = { {"X", true}, {"Y", true} };
	_outputNames = {{"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(SubtractNode, NodeClass::SUBTRACT, 1)

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
	_inputNames = { {"X", true}, {"Y", true} };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ProductNode, NodeClass::PRODUCT, 1)

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
	_inputNames = { {"X", true}, {"Y", true} };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(DivideNode, NodeClass::DIVIDE, 1)

void DivideNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = context.stack[inputs[i]] / context.stack[inputs[i+_channelCount]];
	}
}

ScaleOffsetNode::ScaleOffsetNode(){
	_name = "Scale & Offset";
	_description = "M=A*X+B";
	_inputNames = { {"X", true} };
	_outputNames = { {"M", true } };
	_attributes = { {"A", Attribute::Type::FLOAT}, {"B", Attribute::Type::FLOAT} };
	_attributes[0].flt = 1.0f;
	_attributes[1].flt = 0.f;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ScaleOffsetNode, NodeClass::SCALE_OFFSET, 1)

void ScaleOffsetNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = context.stack[inputs[i]] * _attributes[0].flt + _attributes[1].flt;
	}
}

MinNode::MinNode(){
	_name = "Minimum";
	_description = "M=min(X,Y)";
	_inputNames = { {"X", true}, {"Y", true} };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(MinNode, NodeClass::MINI, 1)

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
	_inputNames = { {"X", true}, {"Y", true} };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(MaxNode, NodeClass::MAXI, 1)

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
	_inputNames = { {"X", true} };
	_outputNames = { {"M", true} };
	_attributes = { {"A", Attribute::Type::FLOAT}, {"B", Attribute::Type::FLOAT} };
	_attributes[0].flt = 0.f;
	_attributes[1].flt = 1.f;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ClampNode, NodeClass::CLAMP, 1)

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
	_inputNames = { {"X", true}, {"Y", true} };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(PowerNode, NodeClass::POWER, 1)

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
	_inputNames = { {"X", true} };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(SqrtNode, NodeClass::SQRT, 1)

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
	_inputNames = { {"X", true} };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ExponentialNode, NodeClass::EXPONENTIAL, 1)

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
	_inputNames = { {"X", true} };
	_outputNames = { {"M", true} };
	_attributes = {{"Basis", Attribute::Type::FLOAT}};
	_attributes[0].flt = glm::e<float>();
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(LogarithmNode, NodeClass::LOGARITHM, 1)

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
	_inputNames = { {"X", true}, {"Y", true}, {"T", true} };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(MixNode, NodeClass::MIX, 1)

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
	_inputNames = { {"X", true } };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(SinNode, NodeClass::SINE, 1)

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
	_inputNames = { {"X", true } };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(CosNode, NodeClass::COSINE, 1)

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
	_inputNames = { {"X", true } };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(TanNode, NodeClass::TANGENT, 1)

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
	_inputNames = { {"X", true } };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ArcSinNode, NodeClass::ARCSINE, 1)

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
	_inputNames = { {"X", true } };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ArcCosNode, NodeClass::ARCCOSINE, 1)

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
	_inputNames = { {"X", true } };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ArcTanNode, NodeClass::ARCTANGENT, 1)

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
	_inputNames = { {"X", true}, {"Y", true} };
	_outputNames = { {"M", false} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(DotProductNode, NodeClass::DOT, 1)

void DotProductNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1);

	float dot = 0.f;
	for(uint i = 0; i < _channelCount; ++i){
		const float y = context.stack[inputs[i + _channelCount]];
		const float x = context.stack[inputs[i]];
		dot += x * y;
	}
	context.stack[outputs[0]] = dot;
}

AbsNode::AbsNode(){
	_name = "Absolute value";
	_description = "M=|X|";
	_inputNames = { {"X", true } };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(AbsNode, NodeClass::ABS, 1)

void AbsNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = std::abs(context.stack[inputs[i]]);
	}
}

FractNode::FractNode(){
	_name = "Fractional part";
	_description = "M=X-\\X/";
	_inputNames = { {"X", true } };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(FractNode, NodeClass::FRACT, 1)

void FractNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::fract(context.stack[inputs[i]]);
	}
}

ModuloNode::ModuloNode(){
	_name = "Modulo";
	_description = "M=X%Y"; 
	_inputNames = { {"X", true}, {"Y", true} };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ModuloNode, NodeClass::MODULO, 1)

void ModuloNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::mod(context.stack[inputs[i]], context.stack[inputs[i+_channelCount]]);
	}
}

FloorNode::FloorNode(){
	_name = "Floor";
	_description = "M=\\X/";
	_inputNames = { {"X", true } };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(FloorNode, NodeClass::FLOOR, 1)

void FloorNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::floor(context.stack[inputs[i]]);
	}
}

CeilNode::CeilNode(){
	_name = "Ceiling";
	_description = "M=/X\\";
	_inputNames = { {"X", true } };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(CeilNode, NodeClass::CEIL, 1)

void CeilNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::ceil(context.stack[inputs[i]]);
	}
}

StepNode::StepNode(){
	_name = "Step";
	_description = "M=if X>A then 1 else 0"; 
	_inputNames = { {"X", true}, {"A", true} };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(StepNode, NodeClass::STEP, 1)

void StepNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 2 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::step(context.stack[inputs[i+_channelCount]], context.stack[inputs[i]]);
	}
}

SmoothstepNode::SmoothstepNode(){
	_name = "Smoothstep";
	_description = "M=smooth transition from 0 to 1 when X goes from A to B";
	_inputNames = { {"X", true}, {"A", true}, {"B", true} };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(SmoothstepNode, NodeClass::SMOOTHSTEP, 1)

void SmoothstepNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 3 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::smoothstep(context.stack[inputs[i+_channelCount]], context.stack[inputs[i+2*_channelCount]], context.stack[inputs[i]]);
	}
}

SignNode::SignNode(){
	_name = "Sign";
	_description = "M=if X > 0 then 1, if X < 0 then -1, if X = 0 then 0";
	_inputNames = { {"X", true } };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(SignNode, NodeClass::SIGN, 1)

void SignNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = glm::sign(context.stack[inputs[i]]);
	}
}

LengthNode::LengthNode(){
	_name = "Length";
	_description = "M=|X|";
	_inputNames = { {"X", true } };
	_outputNames = { {"M", false} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(LengthNode, NodeClass::LENGTH, 1)

void LengthNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1);
	float denom = 0.f;
	for(uint i = 0; i < _channelCount; ++i){
		float comp = context.stack[inputs[i]];
		denom += comp * comp;
	}
	denom = glm::sqrt(denom);
	context.stack[outputs[0]] = denom;
}

NormalizeNode::NormalizeNode(){
	_name = "Normalize";
	_description = "M=X/|X|";
	_inputNames = { {"X", true } };
	_outputNames = { {"M", true} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(NormalizeNode, NodeClass::NORMALIZE, 1)

void NormalizeNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1 * _channelCount);
	assert(outputs.size() == 1 * _channelCount);
	float denom = 0.f;
	for(uint i = 0; i < _channelCount; ++i){
		float comp = context.stack[inputs[i]];
		denom += comp * comp;
	}
	denom = glm::max(1e-3f, glm::sqrt(denom));
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = context.stack[inputs[i]] / denom;
	}
}
