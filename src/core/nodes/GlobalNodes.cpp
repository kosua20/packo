#include "core/nodes/GlobalNodes.hpp"
#include "core/nodes/Nodes.hpp"

// TODO: costly nodes should ignore outputs that go to the dummy register. Could be ignored if 1/4 switch supported.

FlipNode::FlipNode(){
	_name = "Flip";
	_inputNames = {"R", "G", "B", "A"};
	_outputNames = {"R", "G", "B", "A"};
	_attributes = { {"Axis", {"Horizontal", "Vertical"}}};
}

NODE_DEFINE_TYPE_AND_VERSION(FlipNode, NodeClass::FLIP, 1)

void FlipNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(outputs.size() == 4);
	assert(inputs.size() == 4);

	const bool horizontal = _attributes[0].cmb == 0;
	const int x = context.coords.x;
	const int y = context.coords.y;
	const int xOld =  horizontal ? (context.shared->dims.x - x - 1) : x;
	const int yOld = !horizontal ? (context.shared->dims.y - y - 1) : y;

	for(uint i = 0u; i < 4u; ++i){
		const uint srcId = inputs[i];
		const uint dstId = outputs[i];

		const uint imageId = srcId / 4u;
		const uint channelId = srcId % 4u;

		const Image& src = context.shared->tmpImagesRead[imageId];
		context.stack[dstId] = src.pixel(xOld, yOld)[channelId];
	}
}


TileNode::TileNode(){
	_name = "Tile";
	_inputNames = { "R", "G", "B", "A" };
	_outputNames = { "R", "G", "B", "A" };
	_attributes = { {"Scale", Attribute::Type::FLOAT} , {"Offset X", Attribute::Type::FLOAT} , {"Offset Y", Attribute::Type::FLOAT} };
	_attributes[ 0 ].flt = 1.0f;
}

NODE_DEFINE_TYPE_AND_VERSION( TileNode, NodeClass::TILE, 1 )

void TileNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert( outputs.size() == 4 );
	assert( inputs.size() == 4 );

	const float scale = _attributes[ 0 ].flt;
	glm::vec2 offset = glm::vec2(_attributes[ 1 ].flt, _attributes[ 2 ].flt);
	offset *= 0.5f;
	const glm::vec2 size(context.shared->dims);

	glm::vec2 unitCoords = (glm::vec2(context.coords) + 0.5f) / size - 0.5f;
	unitCoords *= scale;
	unitCoords += offset;
	const glm::vec2 coords = glm::fract(unitCoords + 0.5f) * size - 0.5f;

	glm::ivec2 corner = glm::floor(coords);
	const glm::vec2 frac = coords - glm::vec2(corner);
	corner += context.shared->dims;
	const glm::ivec2 c00 = glm::ivec2(corner.x % context.shared->dims.x, corner.y % context.shared->dims.y);
	const glm::ivec2 c11 = glm::ivec2((corner.x+1) % context.shared->dims.x, (corner.y+1) % context.shared->dims.y);

	for(uint i = 0u; i < 4u; ++i){
		const uint srcId = inputs[ i ];
		const uint dstId = outputs[ i ];

		const uint imageId = srcId / 4u;
		const uint channelId = srcId % 4u;

		const Image& src = context.shared->tmpImagesRead[ imageId ];

		const float px0y0 = src.pixel(c00.x, c00.y)[channelId];
		const float px1y0 = src.pixel(c11.x, c00.y)[channelId];
		const float px0y1 = src.pixel(c00.x, c11.y)[channelId];
		const float px1y1 = src.pixel(c11.x, c11.y)[channelId];

		const float value = (1.f - frac.x) * (1.f - frac.y) * px0y0 + (frac.x) * (1.f - frac.y) * px1y0 + (1.f - frac.x) * (frac.y) * px0y1 + (frac.x) * (frac.y) * px1y1;
		context.stack[ dstId ] = value;
	}
}


RotateNode::RotateNode(){
	_name = "Rotate";
	_inputNames = { "R", "G", "B", "A" };
	_outputNames = { "R", "G", "B", "A" };
	_attributes = { {"Anglee", Attribute::Type::FLOAT}};
}

NODE_DEFINE_TYPE_AND_VERSION( RotateNode, NodeClass::ROTATE, 1 )

void RotateNode::evaluate( LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs ) const
{
	assert( outputs.size() == 4 );
	assert( inputs.size() == 4 );

	const float angle = _attributes[ 0 ].flt * glm::pi<float>() / 180.0f;
	const float c = std::cos(angle);
	const float s = std::sin(angle);
	const glm::vec2 size(context.shared->dims);

	// Move to unit space.
	const glm::vec2 unitCoords = (glm::vec2(context.coords) + 0.5f) / size - 0.5f;
	glm::vec2 rotCoords;
	rotCoords.x =  c * unitCoords.x + s * unitCoords.y;
	rotCoords.y = -s * unitCoords.x + c * unitCoords.y;
	const glm::vec2 coords = glm::fract(rotCoords + 0.5f) * size - 0.5f;

	glm::ivec2 corner = glm::floor(coords);
	const glm::vec2 frac = coords - glm::vec2(corner);
	corner += context.shared->dims;
	const glm::ivec2 c00 = glm::ivec2(corner.x % context.shared->dims.x, corner.y % context.shared->dims.y);
	const glm::ivec2 c11 = glm::ivec2((corner.x+1) % context.shared->dims.x, (corner.y+1) % context.shared->dims.y);


	for( uint i = 0u; i < 4u; ++i )
	{
		const uint srcId = inputs[ i ];
		const uint dstId = outputs[ i ];

		const uint imageId = srcId / 4u;
		const uint channelId = srcId % 4u;

		const Image& src = context.shared->tmpImagesRead[ imageId ];

		const float px0y0 = src.pixel(c00.x, c00.y)[channelId];
		const float px1y0 = src.pixel(c11.x, c00.y)[channelId];
		const float px0y1 = src.pixel(c00.x, c11.y)[channelId];
		const float px1y1 = src.pixel(c11.x, c11.y)[channelId];

		const float value = (1.f - frac.x) * (1.f - frac.y) * px0y0 + (frac.x) * (1.f - frac.y) * px1y0 + (1.f - frac.x) * (frac.y) * px0y1 + (frac.x) * (frac.y) * px1y1;
		context.stack[ dstId ] = value;
	}
}

GaussianBlurNode::GaussianBlurNode(){
	_name = "Gaussian Blur";
	_inputNames = {"R", "G", "B", "A"};
	_outputNames = {"R", "G", "B", "A"};
	_attributes = { {"Radius", Attribute::Type::FLOAT}};
}

NODE_DEFINE_TYPE_AND_VERSION(GaussianBlurNode, NodeClass::GAUSSIAN_BLUR, 1)

void GaussianBlurNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(outputs.size() == 4);
	assert(inputs.size() == 4);

	const float radiusFrac = _attributes[0].flt;
	const float sigma = radiusFrac / 3.f;
	const float sigma2 = sigma * sigma;
	const int radius = std::ceil(radiusFrac);
	const float normalization = 1.f / (glm::two_pi<float>() * sigma2);

	glm::uvec4 channelIds;
	const Image* images[4];
	for(uint i = 0u; i < 4u; ++i){
		const uint srcId = inputs[i];
		const uint imageId = srcId / 4u;
		images[i] = &context.shared->tmpImagesRead[imageId];
		channelIds[i] = srcId % 4u;
	}

	glm::vec4 accum { 0.0f};
	float denom = 0.f;
	// No axis separation for now.
	for(int dy = -radius; dy <= radius; ++dy){
		for(int dx = -radius; dx <= radius; ++dx){
			glm::ivec2 dcoords{context.coords.x + dx, context.coords.y + dy};
			dcoords = glm::clamp(dcoords, {0.0f, 0.0f}, context.shared->dims - 1);
			const float weight = normalization * exp(-0.5f/sigma2 * (dx*dx+dy*dy));
			denom += weight;
			for(uint i = 0; i < 4u; ++i){
				accum[i] += weight * images[i]->pixel(dcoords)[channelIds[i]];
			}
		}
	}
	accum /= std::max(denom, 1e-4f);

	for(uint i = 0u; i < 4u; ++i){
		context.stack[outputs[i]] = accum[i];
	}
}
