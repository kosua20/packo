#include "core/nodes/HelperNodes.hpp"
#include "core/nodes/Nodes.hpp"

CommentNode::CommentNode(){
	_name = "Comment";
	_description = "A comment";
	_attributes = { {"##X", Attribute::Type::STRING} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( CommentNode, NodeClass::COMMENT, false, false, 1)

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
	_inputNames = { {"M", true} };
	_outputNames = { {"N", true} };
	_attributes = { {"x", Attribute::Type::FLOAT}, {"y", Attribute::Type::FLOAT}, {"##Color", Attribute::Type::COLOR} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( LogNode, NodeClass::LOG, 1)

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

ResolutionNode::ResolutionNode(){
	_name = "Resolution";
	_description = "Output image(s) resolution.";
	_outputNames = { {"W", false }, {"H", false} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(ResolutionNode, NodeClass::RESOLUTION, 1)

void ResolutionNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 2u);
	(void)inputs;
	context.stack[outputs[0]] = float(context.shared->dims.x);
	context.stack[outputs[1]] = float(context.shared->dims.y);
}


CoordinatesNode::CoordinatesNode()
{
	_name = "Coordinates";
	_description = "Output pixel coordinates.";
	_outputNames = { {"X", false }, {"Y", false} };
	_attributes = { {"Range", {"Unit", "Pixel"}}};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( CoordinatesNode, NodeClass::COORDINATES, 1 )

void CoordinatesNode::evaluate( LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs ) const
{
	assert( inputs.size() == 0u );
	assert( outputs.size() == 2u );
	( void )inputs;
	glm::vec2 coords = context.coords;
	// Normalize if requested
	if(_attributes[ 0 ].cmb == 0){
		const glm::vec2 uvs = ( glm::vec2( coords ) + 0.5f ) / glm::vec2( context.shared->dims );
		coords = uvs;
	}
	context.stack[ outputs[ 0 ] ] = coords[0];
	context.stack[ outputs[ 1 ] ] = coords[1];
}

MathConstantNode::MathConstantNode(){
	_name = "Math constant";
	_description = "if invert then 1/(constant * scale) else (constant*scale)";
	_outputNames = { {"X", true} };
	_attributes = { {"##X", {"pi", "sqrt(pi)", "sqrt(2pi)", "sqrt(pi/2)", "e", "ln(2)", "sqrt(2)", "sqrt(3)", "phi"  } }, {"Scale", Attribute::Type::FLOAT}, {"Invert", Attribute::Type::BOOL} };
	_attributes[1].flt = 1.f;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(MathConstantNode, NodeClass::CONST_MATH, 1)

void MathConstantNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 0u);
	assert(outputs.size() == 1u * _channelCount);
	(void)inputs;

	const float values[] = { glm::pi<float>(), glm::root_pi<float>(), glm::root_two_pi<float>(), glm::root_half_pi<float>(),
		glm::e<float>(), glm::ln_two<float>(), glm::root_two<float>(), glm::root_three<float>(), glm::golden_ratio<float>() };
	assert(sizeof(values) / sizeof(values[0]) == _attributes[0].values.size());

	float value = values[_attributes[0].cmb];
	const float scale = _attributes[1].flt;
	const bool invert = _attributes[2].bln;

	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = invert ? 1.f / (scale * value) : (scale * value);
	}
}

BroadcastNode::BroadcastNode(){
	_name = "Broadcast";
	_description = "Replicate X to Y.";
	_inputNames = { {"X", false} };
	_outputNames = { {"Y", true} };

	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(BroadcastNode, NodeClass::BROADCAST, 1)

void BroadcastNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(inputs.size() == 1u);
	assert(outputs.size() == 1u * _channelCount);

	for(uint i = 0; i < _channelCount; ++i){
		context.stack[outputs[i]] = context.stack[inputs[0]];
	}
}
