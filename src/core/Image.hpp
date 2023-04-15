#pragma once
#include "core/Common.hpp"

struct Image {
public:

	enum class Format {
		PNG, JPEG, BMP, TGA,
	};

	Image() = default;

	Image(uint w, uint h, const glm::vec4 & defaultColor = glm::vec4(0.0f));

	Image(const Image& ) = delete;
	Image& operator=(const Image& ) = delete;

	Image( Image&&) = default;
	Image& operator=( Image&&) = delete;

	bool load(const std::string& path);

	bool save(const std::string& path, Format format);
	
	glm::vec4& pixel(int x, int y) { assert(x >= 0 && x < _w && y >= 0 && y < _h); return _pixels[_w * y + x]; }

	const glm::vec4& pixel(int x, int y) const { assert(x >= 0 && x < _w && y >= 0 && y < _h); return _pixels[_w * y + x]; }

	glm::vec4& pixel( const glm::ivec2& c ) { assert(c.x >= 0 && c.x < _w && c.y >= 0 && c.y < _h); return _pixels[ _w * c.y + c.x ]; }

	const glm::vec4& pixel( const glm::ivec2& c ) const { assert(c.x >= 0 && c.x < _w && c.y >= 0 && c.y < _h); return _pixels[ _w * c.y + c.x ]; }

	uint w() const { return _w; }
	uint h() const { return _h; }

private:

	std::vector<glm::vec4> _pixels;
	unsigned int _w = 0u;
	unsigned int _h = 0u;
};

