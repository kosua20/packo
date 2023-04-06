#include "core/nodes/InOutNodes.hpp"

ConstantFloatNode::ConstantFloatNode(){
	_name = "Constant";
	_outputNames = {"Value"};
	_attributes = { {"X", Attribute::Type::FLOAT} };
}

ConstantRGBANode::ConstantRGBANode(){
	_name = "Constant";
	_outputNames = { "R", "G", "B", "A" };
	_attributes = { {"Val", Attribute::Type::COLOR} };
}

FreeList InputNode::_freeList;

InputNode::InputNode() {
	_index = _freeList.getIndex();
	_name = "Input " + std::to_string(_index);
	_outputNames = {"R", "G", "B", "A"};
}

InputNode::~InputNode(){
	_freeList.returnIndex( _index );
}

FreeList OutputNode::_freeList;

OutputNode::OutputNode() {
	_index = _freeList.getIndex();
	_name = "Output " + std::to_string(_index);
	_inputNames = { "R", "G", "B", "A" };
	_attributes = { {"Prefix", Attribute::Type::STRING}, {"Suffix", Attribute::Type::STRING} };
}

OutputNode::~OutputNode(){
	_freeList.returnIndex( _index );
}
