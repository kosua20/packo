#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"
#include "core/FreeList.hpp"

class ConstantFloatNode : public Node {
public:

	ConstantFloatNode();

	uint type() const override;

	uint version() const override;

	void evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const override;
};

class ConstantRGBANode : public Node {
public:

	ConstantRGBANode();

	uint type() const override;

	uint version() const override;

	void evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const override;
};

class InputNode : public Node {
public:

	InputNode();

	virtual ~InputNode();
	
	uint type() const override;

	uint version() const override;

	void evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const override;

private:
	unsigned int _index{0u};
	static FreeList _freeList;
};

class OutputNode : public Node {
public:
	OutputNode();

	virtual ~OutputNode();

	uint type() const override;

	uint version() const override;

	void evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const override;

	std::string generateFileName(uint batch) const;

private:
	unsigned int _index{0u};
	static FreeList _freeList;
};
