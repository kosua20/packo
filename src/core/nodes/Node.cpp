
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
	str = "";
}

Node::Attribute::Attribute( const std::string& aname, const std::vector<std::string>& avalues ) : name(aname), values(avalues), type( Type::COMBO )
{
	clr = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	flt = 0.f;
	cmb = 0;
	bln = false;
	str = "";
}

Node::Attribute::~Attribute() {}

void Node::setChannelCount(uint c){
	if( !channeled() ){
		_currentInputs = _inputNames;
		_currentOutputs = _outputNames;
		return;
	}

	_channelCount = glm::clamp(c, 1u, 4u);
	// Fast case when one channel.
	if( _channelCount == 1u){
		_currentInputs = _inputNames;
		_currentOutputs = _outputNames;
		return;
	}

	const std::string suffixes[4] = {"R", "G", "B", "A"};

	_currentInputs.clear();
	for(const Node::PinInfos& input : _inputNames){
		if( input.channeled ){
			for( uint i = 0; i < _channelCount; ++i ){
				_currentInputs.push_back( input );
				_currentInputs.back().name += suffixes[ i ];
			}
		} else {
			_currentInputs.push_back(input);
		}
	}

	_currentOutputs.clear();
	for( const Node::PinInfos& output : _outputNames ){
		if( output.channeled ){
			for( uint i = 0; i < _channelCount; ++i ){
				_currentOutputs.push_back( output );
				_currentOutputs.back().name += suffixes[ i ];
			}
		} else {
			_currentOutputs.push_back( output );
		}
	}
}

void Node::serialize(json& data) const {
	data["type"] = type();
	data["version"] = version();
	data["channels"] = channelCount();
	data["attributes"] = {};

	for(const Node::Attribute& att : _attributes ){

		auto& attrData = data["attributes"].emplace_back();
		attrData["name"] = att.name;
		switch (att.type) {
			case Node::Attribute::Type::FLOAT:
				attrData["flt"] = att.flt;
				break;
			case Node::Attribute::Type::COLOR:
			case Node::Attribute::Type::VEC3:
				for(uint colId = 0; colId < 4; ++colId){
					attrData["clr"][colId] = att.clr[colId];
				}
				break;
			case Node::Attribute::Type::STRING:
				attrData["str"] = att.str;
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

	if(data.contains("channels")){
		setChannelCount(data["channels"]);
	} else {
		setChannelCount(1);
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
			case Node::Attribute::Type::VEC3:
			{
				if(attrData.contains("clr")){
					const uint count = std::min(uint(attrData["clr"].size()), 4u);
					for(uint colId = 0; colId < count; ++colId){
						att.clr[colId] = attrData["clr"][colId];
					}
				}
				break;
			}
			case Node::Attribute::Type::STRING:
				if(attrData.contains("str")){
					att.str = attrData["str"];
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

void Node::finalize(){
	_channeled = false;
	for( const auto& input : _inputNames ){
		_channeled = _channeled || input.channeled;
	}
	for(const auto& output : _outputNames){
		_channeled = _channeled || output.channeled;
	}
	_currentInputs = _inputNames;
	_currentOutputs = _outputNames;
	_channelCount = 1;
}
