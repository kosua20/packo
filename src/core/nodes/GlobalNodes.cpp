#include "core/nodes/GlobalNodes.hpp"
#include "core/nodes/Nodes.hpp"

FlipNode::FlipNode(){
	_name = "Flip";
	_inputNames = {"R", "G", "B", "A"};
	_outputNames = {"R", "G", "B", "A"};
	_attributes = { {"Axis", {"Horizontal", "Vertical"}}};
}

NODE_DEFINE_TYPE_AND_VERSION(FlipNode, NodeClass::FLIP, 1)

void FlipNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	const bool horizontal = _attributes[0].cmb == 0;
	// How does this behave for larger inputs?
	const Image& src = context.shared->tmpImages[0];

	const int x = context.coords.x;
	const int y = context.coords.y;
	const int xOld =  horizontal ? (src.w() - x - 1) : x;
	const int yOld = !horizontal ? (src.h() - y - 1) : y;
	for(uint i = 0; i < 4; ++i){
		context.stack[outputs[i]] = src.pixel(xOld, yOld)[i];
	}
}

BackupNode::BackupNode(){
	_name = "Backup";
}

NODE_DEFINE_TYPE_AND_VERSION(BackupNode, NodeClass::INTERNAL_BACKUP, 1)

void BackupNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(outputs.size() == 0u);
	const uint count = inputs.size();
	for(uint i = 0u; i < count; ++i){
		const uint imgId = i / 4u;
		const uint channelId = i % 4u;
		Image& img = context.shared->tmpImages[imgId];
		glm::vec4& pix = img.pixel(context.coords.x, context.coords.y);
		pix[channelId] = context.stack[inputs[i]];
	}
}

RestoreNode::RestoreNode(){
	_name = "Backup";
}

NODE_DEFINE_TYPE_AND_VERSION(RestoreNode, NodeClass::INTERNAL_RESTORE, 1)

void RestoreNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	const uint count = outputs.size();
	for(uint i = 0u; i < count; ++i){
		const uint imgId = i / 4u;
		const uint channelId = i % 4u;
		const Image& img = context.shared->tmpImages[imgId];
		const glm::vec4& pix = img.pixel(context.coords.x, context.coords.y);
		context.stack[outputs[i]] = pix[channelId];
	}
}
