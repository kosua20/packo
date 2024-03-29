#include "core/Image.hpp"
#include "core/system/TextUtilities.hpp"
#define STBI_WINDOWS_UTF8

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image/stb_image_resize.h>

#define TINYEXR_IMPLEMENTATION
#define TINYEXR_USE_MINIZ 0
#define TINYEXR_USE_STB_ZLIB 1
#include <tinyexr/tinyexr.h>

#include <unordered_map>

Image::Image(uint w, uint h, const glm::vec4& defaultColor) {
	_w = w;
	_h = h;
	_pixels.resize(_w * _h, defaultColor);
}

bool Image::load(const fs::path& path){

	const auto dstPathStr = path.u8string();

	if(path.extension() == "exr"){
		float* data = nullptr;
		int wi = 0;
		int hi = 0;
		const char* error = nullptr;
		const int res = LoadEXR(&data, &wi, &hi, dstPathStr.c_str(), &error);
		if(res != TINYEXR_SUCCESS){
			// Should we free the error? doc is unclear.
			return false;
		}
		if(data == nullptr || wi == 0 || hi == 0){
			if(data){
				free(data);
			}
			return false;
		}
		_w = (uint)wi;
		_h = (uint)hi;
		_pixels.resize(_w * _h);
		std::memcpy(_pixels.data(), data, sizeof(glm::vec4) * _w * _h);
		free(data);
		return true;
	}

	// Load image.
	int wi = 0;
	int hi = 0;
	int n;
	
	unsigned char* data = stbi_load( dstPathStr.c_str(), &wi, &hi, &n, 4);
	if(data == nullptr || wi == 0 || hi == 0){
		return false;
	}

	_w = (uint)wi;
	_h = (uint)hi;

	_pixels.resize(_w * _h);
	for (uint y = 0; y < _h; ++y) {
		for (uint x = 0; x < _w; ++x) {
			for (uint c = 0; c < 4; ++c) {
				_pixels[_w * y + x][c] = (float)(data[(_w * y + x) * 4 + c]) / 255.f;
			}
		}
	}

	stbi_image_free(data);
	return true;
}

bool Image::save(const fs::path& path, Format format) const {

	const std::unordered_map<Format, std::string> extensions = {
		{Format::PNG, "png"},
		{Format::BMP, "bmp"},
		{Format::TGA, "tga"},
		{Format::JPEG, "jpg"},
		{Format::EXR, "exr"},
	};

	fs::path dstPath = path;
	dstPath.replace_extension( extensions.at( format ) );
	const auto dstPathStr = dstPath.u8string();

	if(format == Format::EXR){
		const char* err = nullptr;
		const int res = SaveEXR((float*)_pixels.data(), _w, _h, 4, false, dstPathStr.c_str(), &err);
		// Should we free err?
		return res == TINYEXR_SUCCESS;

	}
	// Convert data to LDR
	std::vector<unsigned char> data(_w * _h * 4);
	for (uint y = 0; y < _h; ++y) {
		for (uint x = 0; x < _w; ++x) {
			for (uint c = 0; c < 4; ++c) {
				data[(_w * y + x) * 4 + c] = (unsigned char)glm::clamp(_pixels[_w * y + x][c] * 255.f, 0.f, 255.f);
			}
		}
	}
	int res = -1;
	switch (format) {
		case Format::PNG:
			res = stbi_write_png( dstPathStr.c_str(), _w, _h, 4, data.data(), 4 * _w);
			break;
		case Format::BMP:
			res = stbi_write_bmp( dstPathStr.c_str(), _w, _h, 4, data.data());
			break;
		case Format::TGA:
			res = stbi_write_tga( dstPathStr.c_str(), _w, _h, 4, data.data());
			break;
		case Format::JPEG:
			res = stbi_write_jpg( dstPathStr.c_str(), _w, _h, 4, data.data(), 100 /* quality */);
			break;
		default:
			assert(false);
			break;
	}

	return res == 0;
}

void Image::resize(const glm::ivec2& newRes, Filter filter){
	if(_w == 0 || _h == 0){
		return;
	}

	std::vector<glm::vec4> newPixels(newRes.x * newRes.y);

	bool success = false;
	if(filter == Filter::SMOOTH){
		float* input = &(_pixels[0][0]);
		float* output = &(newPixels[0][0]);

		int res = stbir_resize_float(input, _w, _h , sizeof(glm::vec4) * _w, output, newRes.x, newRes.y, sizeof(glm::vec4) * newRes.x, 4);
		success = (res == 1);
	} else if(filter == Filter::NEAREST){
		const glm::vec2 srcRes(_w, _h);
		for(uint y = 0u; y < uint(newRes.y); ++y){
			for(uint x = 0u; x < uint(newRes.x); ++x){
				const glm::vec2 uv = (glm::vec2(x, y) + 0.5f) / glm::vec2(newRes);
				const glm::vec2 srcCoords = uv * srcRes - 0.5f;
				const glm::ivec2 srcPix = glm::clamp(glm::floor(srcCoords), glm::vec2(0.f), srcRes - 1.f);
				newPixels[y * newRes.x + x] = pixel(srcPix);
			}
		}
		success = true;
	} else {
		assert(false);
	}

	if(success){
		_w = newRes.x;
		_h = newRes.y;
		std::swap(newPixels, _pixels);
	}
}
