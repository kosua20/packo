#pragma once
#include "core/Common.hpp"

struct Image {
public:

	Image() = default;

	Image(uint w, uint h, const glm::vec4 & defaultColor = glm::vec4(0.0f));

	Image(const Image& ) = delete;
	Image& operator=(const Image& ) = delete;

	Image( Image&&) = default;
	Image& operator=( Image&&) = default;

	bool load(const std::string& path);

	bool save(const std::string& path);

	glm::vec4& pixel(int x, int y) { return _pixels[_w * y + x]; }

	const glm::vec4& pixel(int x, int y) const { return _pixels[_w * y + x]; }

	glm::vec4& pixel( const glm::ivec2& c ) { return _pixels[ _w * c.y + c.x ]; }

	const glm::vec4& pixel( const glm::ivec2& c ) const { return _pixels[ _w * c.y + c.x ]; }

	uint w() const { return _w; }
	uint h() const { return _h; }

private:

	std::vector<glm::vec4> _pixels;
	unsigned int _w = 0u;
	unsigned int _h = 0u;
};

