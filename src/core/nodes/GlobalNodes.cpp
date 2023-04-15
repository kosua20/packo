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
	assert( outputs.size() == inputs.size() );

	const bool horizontal = _attributes[0].cmb == 0;
	const int x = context.coords.x;
	const int y = context.coords.y;
	const int xOld =  horizontal ? ( context.shared->dims.x - x - 1 ) : x;
	const int yOld = !horizontal ? ( context.shared->dims.y - y - 1 ) : y;

	const uint count = inputs.size();
	for( uint i = 0u; i < count; ++i ){
		const uint srcId = inputs[ i ];
		const uint dstId = outputs[ i ];

		const uint imageId = srcId / 4u;
		const uint channelId = srcId % 4u;

		const Image& src = context.shared->tmpImages[ imageId ];
		context.stack[ dstId ] = src.pixel( xOld, yOld )[ channelId ];
	}
}
	
	const uint count = inputs.size();
	for(uint i = 0u; i < count; ++i){
		const uint srcId = inputs[i];
		const uint dstId = outputs[i];

		const uint imageId = srcId / 4u;
		const uint channelId = srcId % 4u;

	}
}
