#include "core/nodes/GlobalNodes.hpp"
#include "core/nodes/Nodes.hpp"

FlipNode::FlipNode(){
	_name = "Flip";
	_description = "Flip an image content along the horizontal/vertical axis";
	_inputNames = {"X"};
	_outputNames = {"Y"};
	_attributes = { {"Axis", {"Horizontal", "Vertical"}}};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(FlipNode, NodeClass::FLIP, true, true, 1)

void FlipNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(outputs.size() == _channelCount);
	assert(inputs.size() == _channelCount);

	const bool horizontal = _attributes[0].cmb == 0;
	const int x = context.coords.x;
	const int y = context.coords.y;
	const int xOld = !horizontal ? (context.shared->dims.x - x - 1) : x;
	const int yOld =  horizontal ? (context.shared->dims.y - y - 1) : y;

	for(uint i = 0u; i < _channelCount; ++i){
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
	_description = "Tile an image content at a given scale and offset.";
	_inputNames = {"X"};
	_outputNames = {"Y"};
	_attributes = { {"Scale", Attribute::Type::FLOAT} , {"Offset X", Attribute::Type::FLOAT} , {"Offset Y", Attribute::Type::FLOAT} };
	_attributes[ 0 ].flt = 1.0f;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( TileNode, NodeClass::TILE, true, true, 1 )

void TileNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert( outputs.size() == _channelCount );
	assert( inputs.size() == _channelCount );

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

	for(uint i = 0u; i < _channelCount; ++i){
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
	_description = "Rotate an image content at a certain angle around its center.";
	_inputNames = {"X"};
	_outputNames = {"Y"};
	_attributes = { {"Angle", Attribute::Type::FLOAT}};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( RotateNode, NodeClass::ROTATE, true,true,  1 )

void RotateNode::evaluate( LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs ) const
{
	assert( outputs.size() == _channelCount );
	assert( inputs.size() == _channelCount );

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


	for( uint i = 0u; i < _channelCount; ++i )
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
	_description = "Apply a gaussian of a given radius to an image content.";
	_inputNames = {"X"};
	_outputNames = {"Y"};
	_attributes = { {"Radius", Attribute::Type::FLOAT}};
	_attributes[0].flt = 2.f;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(GaussianBlurNode, NodeClass::GAUSSIAN_BLUR, true, true, 1)

void GaussianBlurNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(outputs.size() == _channelCount);
	assert(inputs.size() == _channelCount);
	assert(_channelCount <= 4u);
	const float radiusFrac = _attributes[0].flt * (context.shared->scale.x + context.shared->scale.y) * 0.5f;
	const float sigma = radiusFrac / 3.f;
	const float sigma2 = sigma * sigma;
	const int radius = (int)std::ceil(radiusFrac) + 1;
	const float normalization = 1.f / (glm::two_pi<float>() * sigma2);

	glm::uvec4 channelIds;
	const Image* images[4];
	for(uint i = 0u; i < _channelCount; ++i){
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
			for(uint i = 0; i < _channelCount; ++i){
				accum[i] += weight * images[i]->pixel(dcoords)[channelIds[i]];
			}
		}
	}
	accum /= std::max(denom, 1e-4f);

	for(uint i = 0u; i < _channelCount; ++i){
		context.stack[outputs[i]] = accum[i];
	}
}

PickerNode::PickerNode(){
	_name = "Color picker";
	_description = "Read the color at a given pixel and broadcast it to all.";
	_inputNames = {"M"};
	_outputNames = {"N"};
	_attributes = { {"x", Attribute::Type::FLOAT},  {"y", Attribute::Type::FLOAT}};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(PickerNode, NodeClass::PICKER, true, true, 1)

void PickerNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(outputs.size() == _channelCount);
	assert(inputs.size() == _channelCount);

	// Everyone will read from the same coords.
	glm::vec2 xy( _attributes[ 0 ].flt, _attributes[ 1 ].flt );
	xy *= context.shared->scale;
	const glm::ivec2 coords = glm::clamp( glm::ivec2(xy), {0, 0}, context.shared->dims - 1);

	for(uint i = 0u; i < _channelCount; ++i){
		const uint srcId = inputs[i];
		const uint dstId = outputs[i];

		const uint imageId = srcId / 4u;
		const uint channelId = srcId % 4u;

		const Image& src = context.shared->tmpImagesRead[imageId];
		context.stack[dstId] = src.pixel(coords)[channelId];
	}
}

FilterNode::FilterNode(){
	_name = "Filter";
	_description = "Apply a 3x3 filter to each pixel image, using its neighborhood.";
	_inputNames = {"X"};
	_outputNames = {"Y"};
	_attributes = { {"##Row0", Attribute::Type::VEC3}, {"##Row1", Attribute::Type::VEC3}, {"##Row2", Attribute::Type::VEC3}};
	_attributes[0].clr = _attributes[1].clr = _attributes[2].clr = glm::vec4(1.0f);
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(FilterNode, NodeClass::FILTER, true, true, 1)

void FilterNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(outputs.size() == _channelCount);
	assert(inputs.size() == _channelCount);


	// Preload images infos.
	glm::uvec4 channelIds;
	const Image* images[4];
	for(uint i = 0u; i < _channelCount; ++i){
		const uint srcId = inputs[i];
		const uint imageId = srcId / 4u;
		images[i] = &context.shared->tmpImagesRead[imageId];
		channelIds[i] = srcId % 4u;
	}

	const glm::mat3 m{ glm::vec3(_attributes[0].clr), glm::vec3(_attributes[1].clr), glm::vec3(_attributes[2].clr)};
	glm::vec4 accum{ 0.0f};

	// No axis separation for now.
	for(int dy = -1; dy <= 1; ++dy){
		for(int dx = -1; dx <= 1; ++dx){
			glm::ivec2 dcoords{context.coords.x + dx, context.coords.y + dy};
			dcoords = glm::clamp(dcoords, {0.0f, 0.0f}, context.shared->dims - 1);
			const float weight = m[dy + 1][dx + 1];
			for(uint i = 0; i < _channelCount; ++i){
				accum[i] += weight * images[i]->pixel(dcoords)[channelIds[i]];
			}
		}
	}
	const float denom = glm::abs(m[0][0]) + glm::abs(m[0][1]) + glm::abs(m[0][2])
					+ glm::abs(m[1][0]) + glm::abs(m[1][1]) + glm::abs(m[1][2])
					+ glm::abs(m[2][0]) + glm::abs(m[2][1]) + glm::abs(m[2][2]);
	accum /= std::max(denom, 1e-4f);

	for(uint i = 0u; i < _channelCount; ++i){
		context.stack[outputs[i]] = accum[i];
	}
}


FloodFillNode::FloodFillNode(){
	_name = "Flood fill";
	_description = "Propagate non empty pixels";
	_inputNames = {"X"};
	_outputNames = {"Y"};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(FloodFillNode, NodeClass::FLOOD_FILL, true, true, 1)

void FloodFillNode::prepare( SharedContext& context, const std::vector<int>& inputs) const {
	assert(outputs.size() == _channelCount);

	const uint w = context.dims.x;
	const uint h = context.dims.y;
	// TODO: Preload images infos?

	// Do stuff
	std::vector<unsigned char> flags(w * h);
	std::vector<int> seeds(w * h);
	std::vector<int> indices0; indices0.reserve(w * h); // Worst case.
	std::vector<int> indices1; indices1.reserve(w * h); // Worst case.

	const glm::ivec2 deltas[] = {{-1,-1}, {0,-1}, { 1,-1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};
	for(uint y = 0; y < h; ++y){
		for(uint x = 0; x < w; ++x){
			for(uint i = 0; i < _channelCount; ++i){
				float c = context.tmpImagesRead[inputs[i]/4u].pixel(x,y)[inputs[i]%4u];
				if(c == 0.f)
					continue;
				const int id = (int)y * (int)w + (int)x;
				flags[id] = 1u;
				seeds[id] = id;
				indices0.push_back(id);
				break;
			}
		}
	}
	while(!indices0.empty()){
		indices1.clear();
		for(int id : indices0){
			// We are on the border.
			// Look at the 9 neighbors
			flags[id] = 2u;
			for(int n = 0; n < 8; ++n){
				const glm::ivec2 coords = glm::ivec2(id % w, id / w) + deltas[n];
				if(coords.x < 0 || coords.y < 0 || coords.x >= (int)w || coords.y >= (int)h){
					continue;
				}
				const int nid = coords.y * w + coords.x;
				if(flags[nid] < 1u){
					flags[nid] = 1u;
					indices1.push_back(nid);
					seeds[nid] = seeds[id];
				}
			}
		}
		std::swap(indices0, indices1);
	}

	// Write to write images.
	for(uint i = 0u; i < _channelCount; ++i){
		const uint srcId = inputs[i];
		const uint imageId = srcId / 4u;
		const uint channelId = srcId % 4u;
		for(uint y = 0; y < h; ++y){
			for(uint x = 0; x < w; ++x){
				const int id = seeds[y * w + x];
				context.tmpImagesWrite[imageId].pixel(x,y)[channelId] = context.tmpImagesRead[imageId].pixel(id %w, id / w)[channelId];
			}
		}
	}
}

void FloodFillNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(outputs.size() == _channelCount);
	assert(inputs.size() == _channelCount);
	// Just transfer from the write inputs (used as temp storage).
	for(uint i = 0u; i < _channelCount; ++i){
		const uint srcId = inputs[i];
		const uint dstId = outputs[i];
		const uint imgId = srcId / 4u;
		const uint channelId = srcId % 4u;
		context.stack[dstId] = context.shared->tmpImagesWrite[imgId].pixel(context.coords)[channelId];
	}
}

MedianFilterNode::MedianFilterNode()
{
	_name = "Median filter";
	_description = "Apply a median filter to each pixel image, using its 3x3 neihborhood";
	_inputNames = { "X" };
	_outputNames = { "Y" };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( MedianFilterNode, NodeClass::MEDIAN_FILTER, true, true, 1 )

void MedianFilterNode::evaluate( LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs ) const
{
	assert( outputs.size() == _channelCount );
	assert( inputs.size() == _channelCount );

	// Preload images infos.
	glm::uvec4 channelIds;
	const Image* images[ 4 ];
	for( uint i = 0u; i < _channelCount; ++i )
	{
		const uint srcId = inputs[ i ];
		const uint imageId = srcId / 4u;
		images[ i ] = &context.shared->tmpImagesRead[ imageId ];
		channelIds[ i ] = srcId % 4u;
	}

	constexpr int kRadius = 1;
	constexpr int kSampleCount = ( 2 * kRadius + 1 ) * ( 2 * kRadius + 1 );

	glm::vec4 values[ kSampleCount ];
	float norms2[ kSampleCount ];
	uint valuesCount = 0u;

	for( int dy = -kRadius; dy <= kRadius; ++dy ){
		for( int dx = -kRadius; dx <= kRadius; ++dx ){
			glm::ivec2 dcoords{ context.coords.x + dx, context.coords.y + dy };
			if( dcoords.x < 0 || dcoords.y < 0 || dcoords.x >= context.shared->dims.x || dcoords.y >= context.shared->dims.y )
				continue;

			glm::vec4 accum(0.f);
			for( uint i = 0; i < _channelCount; ++i ){
				accum[ i ] = images[ i ]->pixel( dcoords )[ channelIds[ i ] ];
			}

			const float norm2 = glm::dot( accum, accum );

			uint insertIndex = 0;
			for( ; insertIndex < valuesCount; ++insertIndex ){
				if( norm2 < norms2[ insertIndex ] ){
					break;
				}
			}
			assert( insertIndex < kSampleCount );
			assert( valuesCount < kSampleCount );
			for(uint nextIndex = valuesCount; nextIndex > insertIndex; --nextIndex ){
				norms2[ nextIndex ] = norms2[ nextIndex - 1 ];
				values[ nextIndex ] = values[ nextIndex - 1 ];
			}
			norms2[ insertIndex ] = norm2;
			values[ insertIndex ] = accum;
			++valuesCount;
		}
	}
	assert( valuesCount > 0 );
	const glm::vec4 medianValue = values[ valuesCount / 2 ];
	for( uint i = 0u; i < _channelCount; ++i ){
		context.stack[ outputs[ i ] ] = medianValue[i];
	}
}


DownscaleNode::DownscaleNode()
{
	_name = "Downscale";
	_description = "Downscale an image and replicate its pixels";
	_inputNames = { "X" };
	_outputNames = { "Y" };
	_attributes = { {"Factor", Attribute::Type::FLOAT} };
	_attributes[ 0 ].flt = 2.f;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( DownscaleNode, NodeClass::DOWNSCALE, true, true, 1 )

void DownscaleNode::evaluate( LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs ) const
{
	assert( outputs.size() == _channelCount );
	assert( inputs.size() == _channelCount );

	const int scale = glm::max(1, int( _attributes[ 0 ].flt ));
	const int sx = ( context.coords.x / scale) * scale;
	const int sy = ( context.coords.y / scale) * scale;

	for( uint i = 0u; i < _channelCount; ++i )
	{
		const uint srcId = inputs[ i ];
		const uint imageId = srcId / 4u;
		const uint channelId = srcId % 4u;

		const float value = context.shared->tmpImagesRead[ imageId ].pixel(sx, sy)[i];
		context.stack[ outputs[ i ] ] = value;
	}
}