
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
		case SUBTRACT:
			return new SubtractNode();
		case PRODUCT:
			return new ProductNode();
		case DIVIDE:
			return new DivideNode();
		case MINI:
			return new MinNode();
		case MAXI:
			return new MaxNode();
		case CLAMP:
			return new ClampNode();
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
		"Subtract", "Product", "Division", "Minimum", "Maximum", "Clamp",
		"Unknown"
	};
	assert(names.size() == NodeClass::COUNT+1);
	return names[uint(type)];
}
