
#include "core/nodes/Node.hpp"
#include <json/json.hpp>

LocalContext::LocalContext(SharedContext* ashared, const glm::vec2& acoords, uint stackSize) : shared(ashared), coords(acoords) {
	stack.resize(stackSize);
}

Node::Attribute::Attribute( const std::string& aname, Type atype ) : name(aname), type( atype )
{
	clr = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	flt = 0.f;
	cmb = 0;
	bln = false;
	memset( str, 0, sizeof( str ) );
}

Node::Attribute::Attribute( const std::string& aname, const std::vector<std::string>& avalues ) : name(aname), values(avalues), type( Type::COMBO )
{
	clr = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	flt = 0.f;
	cmb = 0;
	bln = false;
	memset( str, 0, sizeof( str ) );
}

Node::Attribute::~Attribute() {}

void Node::serialize(json& data) const {
	data["type"] = type();
	data["version"] = version();
	data["attributes"] = {};

	for(const Node::Attribute& att : _attributes ){

		auto& attrData = data["attributes"].emplace_back();
		attrData["name"] = att.name;
		switch (att.type) {
			case Node::Attribute::Type::FLOAT:
				attrData["flt"] = att.flt;
				break;
			case Node::Attribute::Type::COLOR:
				for(uint colId = 0; colId < 4; ++colId){
					attrData["clr"][colId] = att.clr[colId];
				}
				break;
			case Node::Attribute::Type::STRING:
				attrData["str"] = std::string(att.str);
				break;
			case Node::Attribute::Type::COMBO:
				attrData["cmb"] = att.cmb;
				break;
			case Node::Attribute::Type::BOOL:
				attrData["bln"] = att.bln;
				break;
			default:
				assert(false);
				break;
		}
	}
}

bool Node::deserialize(const json& data){
	if(!data.contains("version") || !data.contains("type") || !data.contains("attributes")){
		return false;
	}
	
	if(data["type"] != uint(type())){
		// Wrong type of node.
		return false;
	}
	if(data["version"] > version()){
		// Attempting to open a new node in an old app.
		return false;
	}

	for(auto& attrData : data["attributes"]){
		if(!attrData.contains("name")){
			continue;
		}

		const std::string name = attrData["name"];

		// Find the corresponding attribute (if it stills exists).
		auto attIt = std::find_if(_attributes.begin(), _attributes.end(), [&name](const Attribute& a){
			return a.name == name;
		});
		if(attIt == _attributes.end()){
			continue;
		}
		Attribute& att = *attIt;
		switch(att.type){
			case Node::Attribute::Type::FLOAT:
				if(attrData.contains("flt")){
					att.flt = attrData["flt"];
				}
				break;
			case Node::Attribute::Type::COLOR:
				if(attrData.contains("clr") && attrData["clr"].size() >= 4){
					for(uint colId = 0; colId < 4; ++colId){
						att.clr[colId] = attrData["clr"][colId];
					}
				}
				break;
			case Node::Attribute::Type::STRING:
			{
				if(attrData.contains("str")){
					std::string rawStr = attrData["str"];
					const uint copySize = (std::min)((uint)rawStr.size(), MAX_STR_LENGTH-1);
					memcpy(att.str, rawStr.data(), copySize);
					att.str[copySize] = '\0';
				}
			}
				break;
			case Node::Attribute::Type::COMBO:
				if(attrData.contains("cmb")){
					att.cmb = attrData["cmb"];
					const int lastVal = std::max(int(att.values.size()), 1) - 1;
					att.cmb = glm::clamp(att.cmb, 0, lastVal);
				}
				break;
			case Node::Attribute::Type::BOOL:
				if(attrData.contains("bln")){
					att.bln = attrData["bln"];
				}
				break;
			default:
				assert(false);
				break;
		}
	}
	return true;
}
