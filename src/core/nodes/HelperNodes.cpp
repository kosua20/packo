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


LogNode::LogNode(){
	_name = "Log";
	_description = "Log the color for a given pixel";
	_inputNames = {"M"};
	_outputNames = {"N"};
	_attributes = { {"x", Attribute::Type::FLOAT}, {"y", Attribute::Type::FLOAT}, {"##Color", Attribute::Type::COLOR} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( LogNode, NodeClass::LOG, true, 1)

void LogNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == _channelCount);
	assert(outputs.size() == _channelCount);
	assert(_channelCount <= 4);


	glm::vec2 xy( _attributes[ 0 ].flt, _attributes[ 1 ].flt );
	xy *= context.shared->scale;
	const glm::ivec2 coords = glm::clamp( glm::ivec2( xy ), { 0, 0 }, context.shared->dims - 1 );

	// The pixel will copy its content to the read-only attribute.
	if( coords == context.coords){
		glm::vec4 color(0.0f);
		for(uint i = 0; i < _channelCount; ++i){
			color[i] = context.stack[inputs[i]];
		}
		// Yolo
		const_cast<LogNode*>(this)->_attributes[2].clr = color;
	}

	// Then copy the image to the output
	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = context.stack[inputs[i]];
	}

	const glm::vec2 delta = context.coords - coords;
	const float dist2 = glm::dot(delta, delta);
	const float kRadiusMax = 10.0f;
	const float kRadiusMin = 4.0f;
	if((kRadiusMin < dist2) && (dist2 < kRadiusMax)){
		// Debug display, red circle
		for(uint i = 0; i < _channelCount; ++i){
			context.stack[outputs[i]] = (i == 0 || i == 3) ? 1.0f : 0.0f;
		}
	}

}
