#pragma once

#ifdef _WIN32
#	define NOMINMAX
#endif

#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <cmath>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

typedef unsigned char uchar;
typedef unsigned int  uint;
typedef unsigned long ulong;

#ifdef _WIN32
#	undef near
#	undef far
#	undef ERROR
#endif

#if defined(__APPLE__) && defined(__MACH__)
#	define _MACOS
#endif

#include "system/Logger.hpp"
