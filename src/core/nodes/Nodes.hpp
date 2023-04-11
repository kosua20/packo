#pragma once
#include "core/nodes/Node.hpp"
#include "core/nodes/InOutNodes.hpp"
#include "core/nodes/ArithmeticNodes.hpp"

enum NodeClass : uint {
	INPUT_IMG = 0,
	OUTPUT_IMG,
	ADD,
	CONST_FLOAT,
	CONST_COLOR,
	COUNT
};


Node* createNode(NodeClass type);

const std::string& getNodeName(NodeClass type);
