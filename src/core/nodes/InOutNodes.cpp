#include "core/nodes/InOutNodes.hpp"
#include "core/nodes/Nodes.hpp"

ConstantFloatNode::ConstantFloatNode(){
	_name = "Constant";
	_outputNames = {"Value"};
	_attributes = { {"##X", Attribute::Type::FLOAT} };
}

uint ConstantFloatNode::type() const {
	return NodeClass::CONST_FLOAT;
}

uint ConstantFloatNode::version() const{
	return 1;
}

ConstantRGBANode::ConstantRGBANode(){
	_name = "Constant";
	_outputNames = { "R", "G", "B", "A" };
	_attributes = { {"##Val", Attribute::Type::COLOR} };
}

uint ConstantRGBANode::type() const {
	return NodeClass::CONST_COLOR;
}

uint ConstantRGBANode::version() const{
	return 1;
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

uint InputNode::type() const {
	return NodeClass::INPUT;
}

uint InputNode::version() const{
	return 1;
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

uint OutputNode::type() const {
	return NodeClass::OUTPUT;
}

uint OutputNode::version() const{
	return 1;
}
