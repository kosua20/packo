#pragma once
#include "core/nodes/Node.hpp"
#include "core/nodes/InOutNodes.hpp"
#include "core/nodes/ArithmeticNodes.hpp"
#include "core/nodes/GenerativeNodes.hpp"

enum NodeClass : uint {
	INPUT_IMG = 0,
	OUTPUT_IMG,
	ADD,
	CONST_FLOAT,
	CONST_COLOR,
	SUBTRACT,
	PRODUCT,
	DIVIDE,
	MINI,
	MAXI,
	CLAMP,
	POWER,
	SQRT,
	EXPONENTIAL,
	LOGARITHM,
	COUNT_EXPOSED,
	COUNT
};


Node* createNode(NodeClass type);

const std::string& getNodeName(NodeClass type);
