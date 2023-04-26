#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"

class CommentNode : public Node {
public:

	CommentNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()
};

