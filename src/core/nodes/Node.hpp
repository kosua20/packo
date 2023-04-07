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
			char str[ MAX_STR_LENGTH ]; // TODO: char buffer instead?
			glm::vec4 clr;
			float flt;
		};

		Attribute( const std::string& aname, Type atype ) : name(aname), type( atype )
		{
			switch( type )
			{
				case Type::COLOR:
					clr = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
					break;
				case Type::FLOAT:
					flt = 0.f;
					break;
				case Type::STRING:
					memset( str, 0, sizeof( str ) );
					break;
				default:
					assert( false );
			}
		}
		~Attribute() {};
	};

	virtual bool evaluate( const std::vector<float>& inputs, std::vector<float>& outputs ) { return true; };
	virtual ~Node() = default;
	
	const std::string& name() const { return _name; }
	const std::vector<std::string>& inputNames() const { return _inputNames; }
	const std::vector<std::string>& outputNames() const { return _outputNames; }
	uint inputCount() const { return (uint)_inputNames.size(); }
	uint outputCount() const { return ( uint )_outputNames.size(); }
	uint attributeCount() const { return ( uint )_attributes.size(); }

	std::vector<Attribute>& attributes( )  { return _attributes; }

protected:

	std::string _name;
	std::vector<std::string> _inputNames;
	std::vector<std::string> _outputNames;
	std::vector<Attribute> _attributes;

};
