
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
		case GAUSSIAN_BLUR:
			return new GaussianBlurNode();
		case RANDOM_FLOAT:
			return new UniformRandomNode();
		case RANDOM_COLOR:
			return new RandomColorNode();
		case TILE:
			return new TileNode();
		case ROTATE:
			return new RotateNode();
		case SELECT:
			return new SelectNode();
		case EQUAL:
			return new EqualNode();
		case DIFFERENT:
			return new DifferentNode();
		case NOT:
			return new NegateNode();
		case GREATER:
			return new GreaterNode();
		case LESSER:
			return new LessNode();
		case MIX:
			return new MixNode();
		case COMMENT:
			return new CommentNode();
		case LOG:
			return new LogNode();
		case PICKER:
			return new PickerNode();
		case GRADIENT:
			return new GradientNode();
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
		"Flip", "Gaussian Blur", "Random Scalar", "Random Color", "Tile", "Rotate",
		"Select", "Equal", "Different", "Not", "Greater", "Less", "Interpolate", "Comment", "Log Color", "Pick Color", "Gradient",
		"Internal", "Backup", "Restore",
		"Unknown"
	};
	assert(names.size() == NodeClass::COUNT+1);
	assert(uint(type) < names.size());
	return names[uint(type)];
}

NodeClass getOrderedType(uint i){
	static const std::vector<NodeClass> classes = {
		// Inputs outputs
		INPUT_IMG, OUTPUT_IMG,
		// Scalars
		CONST_FLOAT, RANDOM_FLOAT, GRADIENT,
		// Colors
		CONST_COLOR, RANDOM_COLOR, PICKER,
		// Math
		ADD, SUBTRACT, PRODUCT, DIVIDE, POWER, SQRT, EXPONENTIAL, LOGARITHM,
		MINI, MAXI, CLAMP, MIX,
		// Comparisons
		SELECT, EQUAL, DIFFERENT, NOT, GREATER, LESSER,
		// Global
		FLIP, TILE, ROTATE, GAUSSIAN_BLUR,
		// Helpers
		COMMENT, LOG
	};
	assert(classes.size() == NodeClass::COUNT_EXPOSED);
	assert(i < classes.size());
	return classes[i];
}
