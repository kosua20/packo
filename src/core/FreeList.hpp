#pragma once
#include "core/Common.hpp"
#include <list>

class FreeList {
public:
	
	unsigned int getIndex();

	void returnIndex(unsigned int id);

	size_t size() const { return _freeList.size(); }
	
private:

	std::list<unsigned int> _freeList;
	unsigned int _maxIndex{0u};
};

