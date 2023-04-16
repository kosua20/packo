#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"

class ConstantFloatNode : public Node {
public:

	ConstantFloatNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class ConstantRGBANode : public Node {
public:

	ConstantRGBANode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class UniformRandomNode : public Node {
public:

	UniformRandomNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class RandomColorNode : public Node {
public:

	RandomColorNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};
