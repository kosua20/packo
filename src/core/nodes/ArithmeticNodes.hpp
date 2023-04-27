#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"

class AddNode : public Node {
public:

	AddNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};


class SubtractNode : public Node {
public:

	SubtractNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class ProductNode : public Node {
public:

	ProductNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};


class DivideNode : public Node {
public:

	DivideNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};


class MinNode : public Node {
public:

	MinNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};


class MaxNode : public Node {
public:

	MaxNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class ClampNode : public Node {
public:

	ClampNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class PowerNode : public Node {
public:

	PowerNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class SqrtNode : public Node {
public:

	SqrtNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class ExponentialNode : public Node {
public:

	ExponentialNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class LogarithmNode : public Node {
public:

	LogarithmNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class MixNode : public Node {
public:

	MixNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class SinNode : public Node {
public:

	SinNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class CosNode : public Node {
public:

	CosNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class TanNode : public Node {
public:

	TanNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class ArcSinNode : public Node {
public:

	ArcSinNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class ArcCosNode : public Node {
public:

	ArcCosNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class ArcTanNode : public Node {
public:

	ArcTanNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class DotProductNode : public Node {
public:

	DotProductNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};
