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
	return NodeClass::INPUT_IMG;
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
	return NodeClass::OUTPUT_IMG;
}

uint OutputNode::version() const{
	return 1;
}

std::string OutputNode::generateFileName(uint batch) const {
	const std::string prefix(_attributes[0].str);
	const std::string suffix(_attributes[1].str);
	// Special case if no suffix or prefix
	if(prefix.empty() && suffix.empty()){
		return std::to_string(batch) + "_" + std::to_string(_index);
	}
	// Else don't introduce the output index.
	return prefix + "_" + std::to_string(batch) + "_" + suffix;
}
