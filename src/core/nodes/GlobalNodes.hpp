#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"

class FlipNode : public Node {
public:

	FlipNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

	bool global() const override { return true; }
};

class TileNode : public Node
{
public:

	TileNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

	bool global() const override { return true; }
};

class RotateNode : public Node
{
public:

	RotateNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

	bool global() const override { return true; }
};


class GaussianBlurNode : public Node {
public:

	GaussianBlurNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

	bool global() const override { return true; }
};

class PickerNode : public Node {
public:

	PickerNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

	bool global() const override { return true; }
};

class FilterNode : public Node {
public:

	FilterNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

	bool global() const override { return true; }
};
