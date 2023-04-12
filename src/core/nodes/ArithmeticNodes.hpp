#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"

#include "core/FreeList.hpp"

class AddNode : public Node {
public:

	AddNode();

	NODE_DECLARE_EVAL_TYPE_AND_VERSION()

};

