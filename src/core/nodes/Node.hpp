#pragma once
#include "core/Common.hpp"

class Node {
public:

	virtual bool evaluate( const std::vector<float>& inputs, std::vector<float>& outputs ) { return true; };
	virtual ~Node() = default;
	
	const std::string& name() const { return _name; }
	const std::vector<std::string>& inputNames() const { return _inputNames; }
	const std::vector<std::string>& outputNames() const { return _outputNames; }
	uint inputCount() const { return (uint)_inputNames.size(); }
	uint outputCount() const { return (uint)_outputNames.size();}

protected:

	std::string _name;
	std::vector<std::string> _inputNames;
	std::vector<std::string> _outputNames;

};
