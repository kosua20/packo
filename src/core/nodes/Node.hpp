#pragma once
#include "core/Common.hpp"
#include "core/Image.hpp"

#include <vector>

struct SharedContext {
	std::vector<Image> inputImages;
	std::vector<Image> outputImages;
	std::vector<Image> tmpImagesRead;
	std::vector<Image> tmpImagesWrite;
	std::vector<Image> tmpImagesGlobal;
	glm::ivec2 dims;
	glm::vec2 scale;
};

struct LocalContext {

	LocalContext(SharedContext* ashared, const glm::vec2& acoords, uint stackSize);

	SharedContext* const shared;
	std::vector<float> stack;
	const glm::ivec2 coords;
};

class Node {
public:

	struct Attribute
	{
		enum class Type
		{
			FLOAT, STRING, COLOR, COMBO, BOOL, VEC3
		};

		std::string name;
		std::vector<std::string> values;
		Type type;
		
		std::string str;
		glm::vec4 clr;
		float flt;
		int cmb;
		bool bln;

		Attribute( const std::string& aname, Type atype );

		Attribute( const std::string& aname, const std::vector<std::string>& avalues );

		~Attribute();
	};

	struct PinInfos {
		std::string name = "Unknown pin";
		bool channeled = false;

	};
	
	virtual void prepare( SharedContext& context, const std::vector<int>& inputs) const { (void)context; (void)inputs; assert(global());};

	virtual void evaluate( LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs ) const = 0;

	virtual ~Node() = default;

	virtual void serialize(json& data) const;
	virtual bool deserialize(const json& data);

	virtual uint type() const = 0;
	virtual uint version() const = 0;
	virtual bool global() const { return false; }
	bool channeled() const { return _channeled; }

	void setChannelCount(uint c);
	uint channelCount() const { return _channelCount; }

	const std::string& name() const { return _name; }
	const std::string& description() const { return _description; }
	const std::vector<PinInfos>& inputs() const { return _currentInputs; }
	const std::vector<PinInfos>& outputs() const { return _currentOutputs; }
	const std::vector<Attribute>& attributes( ) const { return _attributes; }
	std::vector<Attribute>& attributes( )  { return _attributes; }

protected:

	void finalize();
	
	std::string _name;
	std::string _description;
	std::vector<PinInfos> _inputNames;
	std::vector<PinInfos> _outputNames;
	std::vector<PinInfos> _currentInputs;
	std::vector<PinInfos> _currentOutputs;
	std::vector<Attribute> _attributes;
	uint _channelCount = 1;
	bool _channeled = false;
};

#define NODE_DECLARE_EVAL_TYPE_AND_VERSION() \
void evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const override; \
uint type() const override; \
uint version() const override;

#define NODE_DEFINE_TYPE_AND_VERSION(C, T, V) \
uint C::type() const { return T; } \
uint C::version() const { return V; }
