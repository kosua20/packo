
#include "core/nodes/Node.hpp"

Node::Attribute::Attribute( const std::string& aname, Type atype ) : name(aname), type( atype )
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

Node::Attribute::~Attribute() {}
