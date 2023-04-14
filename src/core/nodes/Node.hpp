#pragma once
#include "core/Common.hpp"
#include "core/Image.hpp"

#include <vector>

constexpr unsigned int MAX_STR_LENGTH = 256;

struct SharedContext {
	std::vector<Image> inputImages;
	std::vector<Image> outputImages;
	std::vector<Image> tmpImages;
	glm::ivec2 dims;
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
			FLOAT, STRING, COLOR, COMBO
		};

		std::string name;
		std::vector<std::string> values;
		Type type;

		char str[ MAX_STR_LENGTH ];
		glm::vec4 clr;
		float flt;
		int cmb;

		Attribute( const std::string& aname, Type atype );

		Attribute( const std::string& aname, const std::vector<std::string>& avalues );

		~Attribute();
	};

	virtual void evaluate( LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs ) const = 0;
	virtual ~Node() = default;

	virtual void serialize(json& data) const;
	virtual bool deserialize(const json& data);

	virtual uint type() const = 0;
	virtual uint version() const = 0;
	virtual bool global() const { return false; }

	const std::string& name() const { return _name; }
	const std::vector<std::string>& inputs() const { return _inputNames; }
	const std::vector<std::string>& outputs() const { return _outputNames; }
	const std::vector<Attribute>& attributes( ) const { return _attributes; }
	std::vector<Attribute>& attributes( )  { return _attributes; }

protected:

	std::string _name;
	std::vector<std::string> _inputNames;
	std::vector<std::string> _outputNames;
	std::vector<Attribute> _attributes;

};

#define NODE_DECLARE_EVAL_TYPE_AND_VERSION() \
void evaluate(LocalContext& context, const std::vector<int>& inputs, const std::vector<int>& outputs) const override; \
uint type() const override; \
uint version() const override;

#define NODE_DEFINE_TYPE_AND_VERSION(C, T, V) \
uint C::type() const { return T; } \
uint C::version() const { return V; }
