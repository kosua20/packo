#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"
#include "core/FreeList.hpp"

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

	std::string generateFileName(uint batch, Image::Format& format) const;

private:
	unsigned int _index{0u};
	static FreeList _freeList;
};

class BackupNode : public Node {
public:

	BackupNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

};

class RestoreNode : public Node {
public:

	RestoreNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

};
