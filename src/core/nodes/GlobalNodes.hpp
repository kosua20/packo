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

	void prepare( SharedContext& context, const std::vector<int>& inputs) const override;

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

	void prepare( SharedContext& context, const std::vector<int>& inputs) const override;

	bool global() const override { return true; }
};

class FloodFillNode : public Node {
public:

	FloodFillNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

	void prepare( SharedContext& context, const std::vector<int>& inputs) const override;

	bool global() const override { return true; }
};


class MedianFilterNode : public Node
{
public:

	MedianFilterNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

	void prepare( SharedContext& context, const std::vector<int>& inputs) const override;

	bool global() const override { return true; }
};

class QuantizeNode : public Node
{
public:

	QuantizeNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

	void prepare( SharedContext& context, const std::vector<int>& inputs) const override;

	bool global() const override { return true; }
};

class SampleNode : public Node
{
public:

	SampleNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

	void prepare( SharedContext& context, const std::vector<int>& inputs ) const override;

	bool global() const override { return true; }
};