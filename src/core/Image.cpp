#include "core/Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <libs/stb_image/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <libs/stb_image/stb_image_write.h>


Image::Image(uint w, uint h, const glm::vec4& defaultColor) {
	_w = w;
	_h = h;
	_pixels.resize(_w * _h, defaultColor);
}

bool Image::load(const std::string& path){
	
	// Load image.
	int wi = 0;
	int hi = 0;
	int n;
	unsigned char* data = stbi_load(path.c_str(), &wi, &hi, &n, 4);
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

bool Image::save(const std::string& path){
	std::vector<unsigned char> data(_w * _h * 4);
	for (uint y = 0; y < _h; ++y) {
		for (uint x = 0; x < _w; ++x) {
			for (uint c = 0; c < 4; ++c) {
				data[(_w * y + x) * 4 + c] = (unsigned char)glm::clamp(_pixels[_w * y + x][c] * 255.f, 0.f, 255.f);
			}
		}
	}

	const int res = stbi_write_png(path.c_str(), _w, _h, 4, data.data(), _w * 4);
	return res == 0;
}

