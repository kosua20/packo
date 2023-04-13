
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
		case POWER:
			return new PowerNode();
		case SQRT:
			return new SqrtNode();
		case EXPONENTIAL:
			return new ExponentialNode();
		case LOGARITHM:
			return new LogarithmNode();
		case FLIP:
			return new FlipNode();
		default:
			assert(false);
			break;
	}
	return nullptr;
}

const std::string& getNodeName(NodeClass type){
	static const std::vector<std::string> names = {
		"Input image", "Output image", "Add", "Constant Scalar", "Constant Color",
		"Subtract", "Product", "Division", "Minimum", "Maximum", "Clamp", "Power", "Square root", "Exponential", "Logarithm",
		"Flip",
		"Internal", "Unknown"
	};
	assert(names.size() == NodeClass::COUNT+1);
	return names[uint(type)];
}
