#pragma once

#include "core/system/Config.hpp"
#include "core/Common.hpp"

/**
 \brief Performs system basic operations such as directory creation, timing, threading, file picking.
 \ingroup System
 */
class System {
public:

	/** Notify the user by sending a 'Bell' signal. */
	static void ping();

	static std::string loadStringFromFile(const std::string & path);
	
	static bool writeStringToFile(const std::string & str, const std::string & path);

	static bool writeDataToFile(unsigned char * data, size_t size, const std::string & path);

	static std::string timestamp();

	static bool fileExists(const std::string& path);

	
};
