#include "core/nodes/InOutNodes.hpp"
#include "core/nodes/Nodes.hpp"

FreeList InputNode::_freeList;

InputNode::InputNode() {
	_index = _freeList.getIndex();
	_name = "Input " + std::to_string(_index);
	_description = "Load an image from disk.";
	_outputNames = {"R", "G", "B", "A"};
	finalize();
}

InputNode::~InputNode(){
	_freeList.returnIndex( _index );
}

NODE_DEFINE_TYPE_AND_VERSION(InputNode, NodeClass::INPUT_IMG, false, 1)

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
	_description = "Output an image to the disk.";
	_inputNames = { "R", "G", "B", "A" };
	_attributes = { {"Format", {"PNG", "BMP", "JPEG", "TGA"}}, {"Prefix", Attribute::Type::STRING}, {"Suffix", Attribute::Type::STRING} };
	finalize();
}

OutputNode::~OutputNode(){
	_freeList.returnIndex( _index );
}

NODE_DEFINE_TYPE_AND_VERSION(OutputNode, NodeClass::OUTPUT_IMG, false, 1)

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
	std::string prefix(_attributes[1].str);
	std::string suffix(_attributes[2].str);

	static const std::vector<Image::Format> formats = { Image::Format::PNG, Image::Format::BMP, Image::Format::JPEG, Image::Format::TGA };
	const int fmtIndex = glm::clamp(_attributes[0].cmb, 0, int(formats.size()) - 1);
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

BackupNode::BackupNode(){
	_name = "Backup";
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(BackupNode, NodeClass::INTERNAL_BACKUP, false, 1)

void BackupNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(outputs.size() == inputs.size());

	const uint count = inputs.size();
	for(uint i = 0u; i < count; ++i){
		const uint srcId = inputs[i];
		const uint dstId = outputs[i];

		const uint imageId = dstId / 4u;
		const uint channelId = dstId % 4u;
		Image& img = context.shared->tmpImagesWrite[imageId];

		img.pixel(context.coords)[channelId] = context.stack[srcId];
	}
}

RestoreNode::RestoreNode(){
	_name = "Restore";
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(RestoreNode, NodeClass::INTERNAL_RESTORE, false, 1)

void RestoreNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == outputs.size());

	const uint count = outputs.size();
	for(uint i = 0u; i < count; ++i){
		const uint srcId = inputs[i];
		const uint dstId = outputs[i];

		const uint imageId = srcId / 4u;
		const uint channelId = srcId % 4u;
		const Image& img = context.shared->tmpImagesRead[imageId];

		context.stack[dstId] = img.pixel(context.coords)[channelId];
	}
}
