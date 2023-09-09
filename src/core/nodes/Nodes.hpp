#pragma once
#include "core/nodes/Node.hpp"
#include "core/nodes/InOutNodes.hpp"
#include "core/nodes/ArithmeticNodes.hpp"
#include "core/nodes/GenerativeNodes.hpp"
#include "core/nodes/GlobalNodes.hpp"
#include "core/nodes/BooleanNodes.hpp"
#include "core/nodes/HelperNodes.hpp"

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
	FLIP,
	GAUSSIAN_BLUR,
	RANDOM_FLOAT,
	RANDOM_COLOR,
	TILE,
	ROTATE,
	SELECT,
	EQUAL,
	DIFFERENT,
	NOT,
	GREATER,
	LESSER,
	MIX,
	COMMENT,
	LOG,
	PICKER,
	GRADIENT,
	SINE, COSINE, TANGENT,
	ARCSINE, ARCCOSINE, ARCTANGENT,
	DOT,
	FILTER,
	ABS,
	FRACT,
	MODULO,
	FLOOR,
	CEIL,
	STEP,
	SMOOTHSTEP,
	SIGN,
	RESOLUTION,
	CONST_MATH,
	COORDINATES,
	LENGTH,
	NORMALIZE,
	SCALE_OFFSET,
	BROADCAST,
	COUNT_EXPOSED,
	INTERNAL_BACKUP,
	INTERNAL_RESTORE,
	COUNT
};


Node* createNode(NodeClass type);

const std::string& getNodeName(NodeClass type);

NodeClass getOrderedType(uint i);
