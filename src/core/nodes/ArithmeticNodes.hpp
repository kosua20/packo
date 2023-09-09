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

class ScaleOffsetNode : public Node {
public:

	ScaleOffsetNode();

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

class AbsNode : public Node {
public:

	AbsNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class FractNode : public Node {
public:

	FractNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class ModuloNode : public Node {
public:

	ModuloNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class FloorNode : public Node {
public:

	FloorNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class CeilNode : public Node {
public:

	CeilNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class StepNode : public Node {
public:

	StepNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class SmoothstepNode : public Node {
public:

	SmoothstepNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class SignNode : public Node {
public:

	SignNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class LengthNode : public Node {
public:

	LengthNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

class NormalizeNode : public Node {
public:

	NormalizeNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};
