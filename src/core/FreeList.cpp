#include "core/FreeList.hpp"

unsigned int FreeList::getIndex(){
	if(_freeList.empty()){
		return _maxIndex++;
	}
	unsigned int index = _freeList.front();
	_freeList.pop_front();
	// The list remains sorted.
	return index;
}

void FreeList::returnIndex(unsigned int id){
	// Easy case if this was the last element created.
	if(_maxIndex == id + 1){
		_maxIndex = id;
		return;
	}
	// Otherwise, add to free list at the proper location to keep it sorted.
	_freeList.insert(std::upper_bound(_freeList.begin(), _freeList.end(), id), id);
}
