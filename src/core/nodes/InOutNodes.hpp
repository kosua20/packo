#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"
#include "core/FreeList.hpp"

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

class InputNode : public Node {
public:

	InputNode();

	virtual ~InputNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

private:
	unsigned int _index{0u};
	static FreeList _freeList;
};

class OutputNode : public Node {
public:
	OutputNode();

	virtual ~OutputNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

	std::string generateFileName(uint batch) const;

private:
	unsigned int _index{0u};
	static FreeList _freeList;
};
