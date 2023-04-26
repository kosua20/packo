#include "core/nodes/HelperNodes.hpp"
#include "core/nodes/Nodes.hpp"

CommentNode::CommentNode(){
	_name = "Comment";
	_description = "A comment";
	_attributes = { {"##X", Attribute::Type::STRING} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( CommentNode, NodeClass::COMMENT, false, 1)

void CommentNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 0u);
	(void)context;
	(void)inputs;
	(void)outputs;
}
