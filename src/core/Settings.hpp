#pragma once
#include "core/Common.hpp"
#include "core/system/Config.hpp"

class PackoConfig : public Config {
public:

	explicit PackoConfig(const std::vector<std::string> & argv) ;

	// Messages.
	bool version = false;
	bool license = false;
	bool bonus = false;
};
