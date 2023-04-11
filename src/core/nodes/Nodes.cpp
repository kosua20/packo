
#include "core/nodes/Nodes.hpp"

Node* createNode(NodeClass type){
	switch(type){
		case INPUT_IMG:
			return new InputNode();
		case OUTPUT_IMG:
			return new OutputNode();
		case ADD:
			return new AddNode();
		case CONST_FLOAT:
			return new ConstantFloatNode();
		case CONST_COLOR:
			return new ConstantRGBANode();
		case COUNT:
		default:
			assert(false);
			break;
	}
	return nullptr;
}

const std::string& getNodeName(NodeClass type){
	static const std::vector<std::string> names = {
		"Input", "Output", "Add", "Constant Scalar", "Constant Color",
		"Unknown"
	};
	return names[uint(type)];
}
