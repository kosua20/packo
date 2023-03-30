#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"
#include "core/FreeList.hpp"

class ConstantFloatNode : public Node {
public:

	ConstantFloatNode();

	float& value(){ return _value; }

private:
	float _value{0.f};
};

class ConstantRGBANode : public Node {
public:

	ConstantRGBANode();

	glm::vec4& value(){ return _value; }

private:
	glm::vec4 _value{0.f};
};

class InputNode : public Node {
public:

	InputNode();

	virtual ~InputNode();

private:
	unsigned int _index{0u};
	static FreeList _freeList;
};

class OutputNode : public Node {
public:
	OutputNode();

	virtual ~OutputNode();

private:
	unsigned int _index{0u};
	
	std::string _prefix;
	std::string _suffix;

	static FreeList _freeList;
};
