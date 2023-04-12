#include "core/nodes/InOutNodes.hpp"
#include "core/nodes/Nodes.hpp"

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

FreeList InputNode::_freeList;

InputNode::InputNode() {
	_index = _freeList.getIndex();
	_name = "Input " + std::to_string(_index);
	_outputNames = {"R", "G", "B", "A"};
}

InputNode::~InputNode(){
	_freeList.returnIndex( _index );
}

NODE_DEFINE_TYPE_AND_VERSION(InputNode, NodeClass::INPUT_IMG, 1)

void InputNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 4u);
	const Image& inputImg = context.shared->inputImages[_index];
	const glm::vec4& color = inputImg.pixel(context.coords.x, context.coords.y);
	for (uint i = 0u; i < 4u; ++i) {
		context.stack[outputs[i]] = color[i];
	}
}

FreeList OutputNode::_freeList;

OutputNode::OutputNode() {
	_index = _freeList.getIndex();
	_name = "Output " + std::to_string(_index);
	_inputNames = { "R", "G", "B", "A" };
	_attributes = { {"Prefix", Attribute::Type::STRING}, {"Suffix", Attribute::Type::STRING} };
}

OutputNode::~OutputNode(){
	_freeList.returnIndex( _index );
}

NODE_DEFINE_TYPE_AND_VERSION(OutputNode, NodeClass::OUTPUT_IMG, 1)

void OutputNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 4u);
	assert(outputs.size() == 0u);
	Image& outImage = context.shared->outputImages[_index];
	glm::vec4& color = outImage.pixel(context.coords.x, context.coords.y);
	for (uint i = 0u; i < 4u; ++i) {
		color[i] = context.stack[inputs[i]];
	}
}

std::string OutputNode::generateFileName(uint batch) const {
	const std::string prefix(_attributes[0].str);
	const std::string suffix(_attributes[1].str);
	const std::string extension = "png";
	// Special case if no suffix or prefix
	if(prefix.empty() && suffix.empty()){
		return std::to_string(batch) + "_" + std::to_string(_index) + "." + extension;
	}
	// Else don't introduce the output index.
	return prefix + "_" + std::to_string(batch) + "_" + suffix + "." + extension;
}
