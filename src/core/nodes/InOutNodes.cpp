#include "core/nodes/InOutNodes.hpp"
#include "core/nodes/Nodes.hpp"

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
	const glm::vec4& color = inputImg.pixel(context.coords);
	for (uint i = 0u; i < 4u; ++i) {
		context.stack[outputs[i]] = color[i];
	}
}

FreeList OutputNode::_freeList;

OutputNode::OutputNode() {
	_index = _freeList.getIndex();
	_name = "Output " + std::to_string(_index);
	_inputNames = { "R", "G", "B", "A" };
	_attributes = { {"Prefix", Attribute::Type::STRING}, {"Suffix", Attribute::Type::STRING}, {"Format", {"PNG", "BMP", "JPEG", "TGA"}} };
}

OutputNode::~OutputNode(){
	_freeList.returnIndex( _index );
}

NODE_DEFINE_TYPE_AND_VERSION(OutputNode, NodeClass::OUTPUT_IMG, 1)

void OutputNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 4u);
	assert(outputs.size() == 0u);
	Image& outImage = context.shared->outputImages[_index];
	glm::vec4& color = outImage.pixel(context.coords);
	for (uint i = 0u; i < 4u; ++i) {
		color[i] = context.stack[inputs[i]];
	}
}

std::string OutputNode::generateFileName(uint batch, Image::Format& format) const {
	std::string prefix(_attributes[0].str);
	std::string suffix(_attributes[1].str);

	static const std::vector<Image::Format> formats = { Image::Format::PNG, Image::Format::BMP, Image::Format::JPEG, Image::Format::TGA };
	const int fmtIndex = glm::clamp(_attributes[2].cmb, 0, int(formats.size()) - 1);
	format = formats[fmtIndex];

	// Special case if no suffix or prefix
	if(prefix.empty() && suffix.empty()){
		return std::to_string(batch) + "_" + std::to_string(_index);
	}
	// Else don't introduce the output index.
	if(!prefix.empty()){
		prefix = prefix + "_";
	}
	if(!suffix.empty()){
		suffix = "_" + suffix;
	}
	return prefix + std::to_string(batch) + suffix;
}
