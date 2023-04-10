#pragma once
#include "core/nodes/Node.hpp"
#include "core/nodes/InOutNodes.hpp"
#include "core/nodes/ArithmeticNodes.hpp"

enum NodeClass : uint {
	INPUT = 0,
	OUTPUT,
	ADD,
	CONST_FLOAT,
	CONST_COLOR,
	COUNT
};
