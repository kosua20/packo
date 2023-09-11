#pragma once
#include "core/Common.hpp"
#include "core/system/System.hpp"

struct Image {
public:

	enum class Format {
		PNG, JPEG, BMP, TGA, EXR
	};

	Image() = default;

	Image(uint w, uint h, const glm::vec4 & defaultColor = glm::vec4(0.0f));

	Image(const Image& ) = delete;
	Image& operator=(const Image& ) = delete;

	Image( Image&&) = default;
	Image& operator=( Image&&) = delete;

	bool load(const fs::path& path);

	bool save(const fs::path& path, Format format) const;

	void resize(const glm::ivec2& newRes);
	
	glm::vec4& pixel(int x, int y) { assert(x >= 0 && x < int(_w) && y >= 0 && y < int(_h)); return _pixels[_w * y + x]; }

	const glm::vec4& pixel(int x, int y) const { assert(x >= 0 && x < int(_w) && y >= 0 && y < int(_h)); return _pixels[_w * y + x]; }

	glm::vec4& pixel( const glm::ivec2& c ) { assert(c.x >= 0 && c.x < int(_w) && c.y >= 0 && c.y < int(_h)); return _pixels[ _w * c.y + c.x ]; }

	const glm::vec4& pixel( const glm::ivec2& c ) const { assert(c.x >= 0 && c.x < int(_w) && c.y >= 0 && c.y < int(_h)); return _pixels[ _w * c.y + c.x ]; }

	uint w() const { return _w; }
	uint h() const { return _h; }

	float* rawPixels() { return (_w*_h == 0) ? nullptr : &( _pixels[ 0 ][ 0 ] ); }
private:

	std::vector<glm::vec4> _pixels;
	unsigned int _w = 0u;
	unsigned int _h = 0u;
};

