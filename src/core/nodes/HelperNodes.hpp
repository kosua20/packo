#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"

class CommentNode : public Node {
public:

	CommentNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class LogNode : public Node {
public:

	LogNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class ResolutionNode : public Node {
public:

	ResolutionNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class CoordinatesNode : public Node
{
public:

	CoordinatesNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class MathConstantNode : public Node {
public:

	MathConstantNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class BroadcastNode : public Node {
public:

	BroadcastNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};



