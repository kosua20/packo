// Include before header including the ghc forward declarations.
#define GHC_FILESYSTEM_IMPLEMENTATION
#include <ghc/filesystem.hpp>

#include "core/system/System.hpp"

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <io.h>
#else
#include <unistd.h>
#endif
#include <iomanip>

void System::ping() {
	Log::Info() << '\a' << std::endl;
}

std::string System::loadStringFromFile(const fs::path& path){
	std::ifstream file(path);
	if(file.bad() || file.fail()) {
		Log::Error() << "Unable to load file at path \"" << path.string() << "\"." << std::endl;
		return "";
	}
	std::stringstream buffer;
	// Read the stream in a buffer.
	buffer << file.rdbuf();
	file.close();
	// Create a string based on the content of the buffer.
	std::string line = buffer.str();
	return line;
}

bool System::writeStringToFile(const std::string & str, const fs::path & path){
	std::ofstream file(path);
	if(file.bad() || file.fail()) {
		Log::Error() << "Unable to write to file at path \"" << path.string() << "\"." << std::endl;
		return false;
	}
	file << str << std::endl;
	file.close();
	return true;
}

bool System::writeDataToFile(unsigned char * data, size_t size, const fs::path & path) {
	std::ofstream outputFile(path, std::ios::binary);

	if(!outputFile.is_open()) {
		Log::Error() << "Unable to save file at path \"" << path.string() << "\"." << std::endl;
		return false;
	}
	outputFile.write(reinterpret_cast<char*>(data), size);
	outputFile.close();
	return true;
}

std::string System::timestamp(){
	const auto time = std::time(nullptr);
#ifdef _WIN32
	tm ltime = { 0,0,0,0,0,0,0,0,0 };
	localtime_s(&ltime, &time);
#else
	const tm ltime = *(std::localtime(&time));
#endif
	std::stringstream str;
	str << std::put_time(&ltime, "%Y_%m_%d_%H_%M_%S");
	return str.str();
}
