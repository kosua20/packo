#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"

#include "core/FreeList.hpp"

class AddNode : public Node {
public:

	AddNode();

	uint type() const override;

	uint version() const override;
	
	void evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const override;
};
