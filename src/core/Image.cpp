#include "core/Image.hpp"
#include "core/system/TextUtilities.hpp"
#define STBI_WINDOWS_UTF8

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image/stb_image_resize.h>

#include <unordered_map>

Image::Image(uint w, uint h, const glm::vec4& defaultColor) {
	_w = w;
	_h = h;
	_pixels.resize(_w * _h, defaultColor);
}

bool Image::load(const fs::path& path){

	const auto dstPathStr = path.u8string();
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
	};

	fs::path dstPath = path;
	dstPath.replace_extension( extensions.at( format ) );

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
	const std::string dstPathStr = dstPath.string();
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

void Image::resize(const glm::ivec2& newRes){
	if(_w == 0 || _h == 0){
		return;
	}

	std::vector<glm::vec4> newPixels(newRes.x * newRes.y);
	float* input = &(_pixels[0][0]);
	float* output = &(newPixels[0][0]);
	int res = stbir_resize_float(input, _w, _h , sizeof(glm::vec4) * _w, output, newRes.x, newRes.y, sizeof(glm::vec4) * newRes.x, 4);
	if(res == 1){
		// Success
		_w = newRes.x;
		_h = newRes.y;
		std::swap(newPixels, _pixels);
	}

}
