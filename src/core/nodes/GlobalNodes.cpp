#include "core/nodes/GlobalNodes.hpp"
#include "core/nodes/Nodes.hpp"


void copyInputsToImage(const std::vector<Image>& srcs, const std::vector<int>& inputs, Image& dst){
	const uint channelCount = (uint)glm::min(4ull, inputs.size());
	for(uint i = 0u; i < channelCount; ++i){
		const uint srcId = inputs[i];
		const uint imageId = srcId / 4u;
		const uint channelId = srcId % 4u;
		const Image& src = srcs[imageId];
		for(uint y = 0; y < dst.h(); ++y){
			for(uint x = 0; x < dst.w(); ++x){
				dst.pixel(x, y)[i] = src.pixel(x,y)[channelId];
			}
		}
	}
	for(uint i = channelCount; i < 4u; ++i){
		for(uint y = 0; y < dst.h(); ++y){
			for(uint x = 0; x < dst.w(); ++x){
				dst.pixel(x, y)[i] = 0.f;
			}
		}
	}
}

FlipNode::FlipNode(){
	_name = "Flip";
	_description = "Flip an image content along the horizontal/vertical axis";
	_inputNames = { {"X", true} };
	_outputNames = { {"Y", true} };
	_attributes = { {"Axis", {"Horizontal", "Vertical"}}};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(FlipNode, NodeClass::FLIP, 1)

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
	_inputNames = { {"X", true} };
	_outputNames = { {"Y", true} };
	_attributes = { {"Scale", Attribute::Type::FLOAT} , {"Offset X", Attribute::Type::FLOAT} , {"Offset Y", Attribute::Type::FLOAT} };
	_attributes[ 0 ].flt = 1.0f;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( TileNode, NodeClass::TILE, 1 )

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
	_inputNames = { {"X", true} };
	_outputNames = { {"Y", true} };
	_attributes = { {"Angle", Attribute::Type::FLOAT}};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( RotateNode, NodeClass::ROTATE, 1 )

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
	_inputNames = { {"X", true} };
	_outputNames = { {"Y", true} };
	_attributes = { {"Radius", Attribute::Type::FLOAT}};
	_attributes[0].flt = 2.f;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(GaussianBlurNode, NodeClass::GAUSSIAN_BLUR, 1)

void GaussianBlurNode::prepare(SharedContext& context, const std::vector<int>& inputs) const {
	assert(inputs.size() == _channelCount);
	assert(inputs.size() <= 4);

	copyInputsToImage(context.tmpImagesRead, inputs, context.tmpImagesGlobal[0]);
}

void GaussianBlurNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(outputs.size() == _channelCount);
	assert(inputs.size() == _channelCount);
	assert(_channelCount <= 4u);
	const float radiusFrac = _attributes[0].flt * (context.shared->scale.x + context.shared->scale.y) * 0.5f;
	const float sigma = radiusFrac / 3.f;
	const float sigma2 = sigma * sigma;
	const int radius = (int)std::ceil(radiusFrac) + 1;
	const float normalization = 1.f / (glm::two_pi<float>() * sigma2);

	const Image& src = context.shared->tmpImagesGlobal[0];
	glm::vec4 accum { 0.0f};
	float denom = 0.f;
	// No axis separation for now.
	for(int dy = -radius; dy <= radius; ++dy){
		for(int dx = -radius; dx <= radius; ++dx){
			glm::ivec2 dcoords{context.coords.x + dx, context.coords.y + dy};
			dcoords = glm::clamp(dcoords, {0.0f, 0.0f}, context.shared->dims - 1);
			const float weight = normalization * exp(-0.5f/sigma2 * (dx*dx+dy*dy));
			denom += weight;
			accum += weight * src.pixel(dcoords);
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
	_inputNames = { {"M", true} };
	_outputNames = { {"N", true} };
	_attributes = { {"x", Attribute::Type::FLOAT},  {"y", Attribute::Type::FLOAT}};
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(PickerNode, NodeClass::PICKER, 1)

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
	_inputNames = { {"X", true} };
	_outputNames = { {"Y", true} };
	_attributes = { {"##Row0", Attribute::Type::VEC3}, {"##Row1", Attribute::Type::VEC3}, {"##Row2", Attribute::Type::VEC3}};
	_attributes[0].clr = _attributes[1].clr = _attributes[2].clr = glm::vec4(1.0f);
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(FilterNode, NodeClass::FILTER, 1)

void FilterNode::prepare(SharedContext& context, const std::vector<int>& inputs) const {
	assert(inputs.size() == _channelCount);
	assert(inputs.size() <= 4);

	copyInputsToImage(context.tmpImagesRead, inputs, context.tmpImagesGlobal[0]);
}

void FilterNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(outputs.size() == _channelCount);
	assert(inputs.size() == _channelCount);
	const Image& src = context.shared->tmpImagesGlobal[0];

	const glm::mat3 m{ glm::vec3(_attributes[0].clr), glm::vec3(_attributes[1].clr), glm::vec3(_attributes[2].clr)};
	glm::vec4 accum{ 0.0f};

	// No axis separation for now.
	for(int dy = -1; dy <= 1; ++dy){
		for(int dx = -1; dx <= 1; ++dx){
			glm::ivec2 dcoords{context.coords.x + dx, context.coords.y + dy};
			dcoords = glm::clamp(dcoords, {0.0f, 0.0f}, context.shared->dims - 1);
			const float weight = m[dy + 1][dx + 1];
			accum += weight * src.pixel(dcoords);
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
	_description = "For each pixel, find the coordinates of the closest non-zero pixel in X";
	_inputNames = { {"X", false} };
	_outputNames = { {"U", false}, {"V", false} };
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION(FloodFillNode, NodeClass::FLOOD_FILL, 1)

void FloodFillNode::prepare( SharedContext& context, const std::vector<int>& inputs) const {
	assert(inputs.size() == 1);

	const uint w = context.dims.x;
	const uint h = context.dims.y;

	std::vector<unsigned char> flags(w * h);
	std::vector<int> seeds(w * h);
	std::vector<int> indices0; indices0.reserve(w * h); // Worst case.
	std::vector<int> indices1; indices1.reserve(w * h); // Worst case.

	const uint srcId = inputs[0];
	const uint imageId = srcId / 4u;
	const uint channelId = srcId % 4u;
	const Image& src = context.tmpImagesRead[imageId];

	// Collect seeds.
	for(uint y = 0; y < h; ++y){
		for(uint x = 0; x < w; ++x){
			if(src.pixel(x,y)[channelId] == 0.f)
				continue;
			const int id = (int)y * (int)w + (int)x;
			flags[id] = 1u;
			seeds[id] = id;
			indices0.push_back(id);
		}
	}

	const glm::ivec2 deltas[] = {{-1,-1}, {0,-1}, { 1,-1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};
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

	// Write to tmp storage for global nodes.
	Image& dst = context.tmpImagesGlobal[0];
	for(uint y = 0; y < h; ++y){
		for(uint x = 0; x < w; ++x){
			const int id = seeds[y * w + x];
			const glm::ivec2 pixCoords(id % w, id / w);
			const glm::vec2 uvs = (glm::vec2(pixCoords) + 0.5f)/ glm::vec2(context.dims);
			dst.pixel(x, y).x = uvs.x;
			dst.pixel(x, y).y = uvs.y;
		}
	}
}

void FloodFillNode::evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const {
	assert(outputs.size() == 2);
	assert(inputs.size() == 1);

	const Image& uvMap = context.shared->tmpImagesGlobal[0];
	const glm::vec2 uvs = glm::vec2(uvMap.pixel(context.coords));
	context.stack[outputs[0]] = uvs.x;
	context.stack[outputs[1]] = uvs.y;
}

MedianFilterNode::MedianFilterNode(){
	_name = "Median filter";
	_description = "Apply a median filter to each pixel of X, only for pixels in mask M";
	_inputNames = { {"X", false}, {"M", false} };
	_outputNames = { {"Y", false} };
	_attributes = { {"Radius", Attribute::Type::FLOAT} };
	_attributes[ 0 ].flt = 1.f;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( MedianFilterNode, NodeClass::MEDIAN_FILTER, 1 )


void MedianFilterNode::prepare(SharedContext& context, const std::vector<int>& inputs) const {
	assert(inputs.size() == 2);
	// Second channel is used as mask.
	copyInputsToImage(context.tmpImagesRead, inputs, context.tmpImagesGlobal[0]);
}

void MedianFilterNode::evaluate( LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs ) const {
	assert(outputs.size() == 1);
	assert(inputs.size() == 2);

	const int kRadius = uint(std::max(0.f, _attributes[ 0 ].flt));
	const int kSampleCount = ( 2 * kRadius + 1 ) * ( 2 * kRadius + 1 );

	std::vector<float> values; 
	values.reserve( kSampleCount );

	const Image& src = context.shared->tmpImagesGlobal[0];
	for(int dy = -kRadius; dy <= kRadius; ++dy){
		for(int dx = -kRadius; dx <= kRadius; ++dx){
			glm::ivec2 dcoords{ context.coords.x + dx, context.coords.y + dy };
			if(dcoords.x < 0 || dcoords.y < 0 || dcoords.x >= context.shared->dims.x || dcoords.y >= context.shared->dims.y)
				continue;

			glm::vec2 value = glm::vec2(src.pixel(dcoords));

			if(std::abs(value.y) < 1e-3f)
				continue;

			const uint valuesCount = (uint)values.size();
			uint insertIndex = 0u;
			for( ; insertIndex < valuesCount; ++insertIndex ){
				if(value.x < values[insertIndex]){
					break;
				}
			}

			assert(insertIndex < (uint)kSampleCount);
			assert(valuesCount < (uint)kSampleCount);
			values.insert( values.begin() + insertIndex, value.x );
		}
	}
	float medianValue = values.empty() ? src.pixel(context.coords).x : values[ values.size() / 2 ];
	context.stack[ outputs[ 0 ] ] = medianValue;
}


QuantizeNode::QuantizeNode(){
	_name = "Quantize";
	_description = "Quantize an image at a given pixel scale";
	_inputNames = { {"X", true} };
	_outputNames = { {"Y", true} };
	_attributes = { {"Factor", Attribute::Type::FLOAT}, {"Bilinear", Attribute::Type::BOOL} };
	_attributes[0].flt = 2.f;
	_attributes[1].bln = false;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( QuantizeNode, NodeClass::QUANTIZE, 1 )

void QuantizeNode::prepare(SharedContext& context, const std::vector<int>& inputs) const {
	assert(inputs.size() == _channelCount);
	assert(inputs.size() <= 4);

	copyInputsToImage(context.tmpImagesRead, inputs, context.tmpImagesGlobal[0]);
}

void QuantizeNode::evaluate( LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs ) const
{
	assert( outputs.size() == _channelCount );
	assert( inputs.size() == _channelCount );

	const bool bilinearUpscale = _attributes[1].bln;
	const int scale = glm::max(1, int( _attributes[ 0 ].flt ));
	// /!\ Two consecutive downscales won't compose, because we re-upscale each time
	// (we duplicate the value in all pixels that project to the same downscaled pixel)
	const int sx = ( context.coords.x / scale) * scale;
	const int sy = ( context.coords.y / scale) * scale;

	const Image& src = context.shared->tmpImagesGlobal[0];

	glm::vec4 basePixel = src.pixel(sx, sy);
	if(bilinearUpscale){
		const glm::vec4 c00 = basePixel;
		const int sx1 = (sx + scale) < context.shared->dims.x ? (sx + scale) : sx;
		const int sy1 = (sy + scale) < context.shared->dims.y ? (sy + scale) : sy;
		const glm::vec4& c10 = src.pixel(sx1,  sy);
		const glm::vec4& c01 = src.pixel( sx, sy1);
		const glm::vec4& c11 = src.pixel(sx1, sy1);
		const glm::vec2 frac = glm::vec2(context.coords - glm::ivec2(sx, sy)) / (float)scale;
		basePixel = (1.f - frac.x) * (1.f - frac.y) * c00 + (1.f - frac.x) * frac.y * c01 + frac.x * (1.f - frac.y) * c10 + frac.x * frac.y * c11;
	}

	for(uint i = 0u; i < _channelCount; ++i){
		context.stack[outputs[i]] = basePixel[i];
	}

}

SampleNode::SampleNode(){
	_name = "Sample";
	_description = "Sample an image at the given UV";
	_inputNames = { {"X", true}, {"U", false}, {"V", false} };
	_outputNames = { {"Y", true} };
	_attributes = { {"Bilinear", Attribute::Type::BOOL} };
	_attributes[ 0 ].bln = false;
	finalize();
}

NODE_DEFINE_TYPE_AND_VERSION( SampleNode, NodeClass::SAMPLING, 1 )

void SampleNode::prepare( SharedContext& context, const std::vector<int>& inputs ) const {
	assert(inputs.size() == _channelCount + 2);

	copyInputsToImage(context.tmpImagesRead, inputs, context.tmpImagesGlobal[ 0 ]);
}

void SampleNode::evaluate( LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs ) const
{
	assert( inputs.size() == _channelCount + 2 );
	assert( outputs.size() == _channelCount );

	const Image& src = context.shared->tmpImagesGlobal[ 0 ];

	// Retrieve UVs.
	// TODO: FIXME
	glm::vec2 coords( 0.f );
	{
		const uint srcIdX = inputs[ _channelCount - 2 ];
		const uint imageIdX = srcIdX / 4u;
		const uint channelIdX = srcIdX % 4u;
		coords.x = context.shared->tmpImagesRead[ imageIdX ].pixel(context.coords)[channelIdX];

		const uint srcIdY = inputs[ _channelCount - 1 ];
		const uint imageIdY = srcIdY / 4u;
		const uint channelIdY = srcIdY % 4u;
		coords.y = context.shared->tmpImagesRead[ imageIdY ].pixel( context.coords )[ channelIdY ];
	}
	coords = glm::fract( coords );
	// Convert to texel coordinates (for now, naive, don't take into account half pixel offset)
	const glm::vec2 imageSize = glm::vec2(context.shared->dims);
	const glm::ivec2 safeSize = context.shared->dims - glm::ivec2(1, 1);

	const glm::vec2 pixCoords = coords * imageSize - 0.5f;
	// We already wrapped above, clamp.
	glm::ivec2 coords00 = glm::ivec2(glm::floor(pixCoords));
	coords00 = glm::clamp(coords00, {0, 0}, safeSize);

	glm::vec4 basePixel = src.pixel(coords00);
	const bool bilinear = _attributes[ 0 ].bln;
	if(bilinear){
		const glm::vec4 c00 = basePixel;
		const glm::ivec2 coords11 = glm::min(coords00 + 1, safeSize);
		const glm::vec4& c10 = src.pixel(coords11.x, coords00.y);
		const glm::vec4& c01 = src.pixel(coords00.x, coords11.y);
		const glm::vec4& c11 = src.pixel(coords11.x, coords11.y);
		const glm::vec2 frac = pixCoords - glm::vec2(coords00);
		basePixel = (1.f - frac.x) * (1.f - frac.y) * c00 + (1.f - frac.x) * frac.y * c01 + frac.x * (1.f - frac.y) * c10 + frac.x * frac.y * c11;
	}

	for( uint i = 0; i < _channelCount; ++i ){
		context.stack[ outputs[ i ] ] = basePixel[i];
	}
}
