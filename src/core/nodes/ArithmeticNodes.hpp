#pragma once
#include "core/Common.hpp"
#include "core/nodes/Node.hpp"

#include "core/FreeList.hpp"

class AddNode : public Node {
public:

	AddNode();

	~AddNode();
	
private:
	uint _index;
	static FreeList _freeList;
};
