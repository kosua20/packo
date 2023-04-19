#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"


class SelectNode : public Node {
public:

	SelectNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};


class EqualNode : public Node {
public:

	EqualNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class DifferentNode : public Node {
public:

	DifferentNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};


class GreaterNode : public Node {
public:

	GreaterNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};


class LessNode : public Node {
public:

	LessNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};


class NegateNode : public Node {
public:

	NegateNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};
