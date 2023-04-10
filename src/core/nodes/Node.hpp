#pragma once
#include "core/Common.hpp"

constexpr unsigned int MAX_STR_LENGTH = 256;

class Node {
public:

	struct Attribute
	{
		enum class Type
		{
			FLOAT, STRING, COLOR
		};

		std::string name;
		Type type;
		union
		{
			char str[ MAX_STR_LENGTH ];
			glm::vec4 clr;
			float flt;
		};

		Attribute( const std::string& aname, Type atype );

		~Attribute();
	};

	virtual bool evaluate( const std::vector<float>& inputs, std::vector<float>& outputs ) { return true; };
	virtual ~Node() = default;

	virtual void serialize(json& data) const;
	virtual bool deserialize(const json& data);

	virtual uint type() const = 0;
	virtual uint version() const = 0;
	
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
