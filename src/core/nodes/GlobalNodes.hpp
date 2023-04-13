#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"

class FlipNode : public Node {
public:

	FlipNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

	bool global() const override { return true; }
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

