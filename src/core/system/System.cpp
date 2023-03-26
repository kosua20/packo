#include "core/system/System.hpp"

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <io.h>
#else
#include <unistd.h>
#endif
#include <iomanip>

#ifdef _WIN32

std::wstring widen(const std::string & str) {
	const int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	WCHAR * arr	= new WCHAR[size];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, static_cast<LPWSTR>(arr), size);
	std::wstring res( arr );
	delete[] arr;
	return res;
}

std::string narrow(WCHAR * str) {
	const int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
	std::string res(size - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, str, -1, &res[0], size, nullptr, nullptr);
	return res;
}

#else

std::string widen(const std::string & str) {
	return str;
}
std::string narrow(char * str) {
	return std::string(str);
}

#endif


void System::ping() {
	Log::Info() << '\a' << std::endl;
}

std::string System::loadStringFromFile(const std::string & path){
	std::ifstream file(widen(path));
	if(file.bad() || file.fail()) {
		Log::Error() << "Unable to load file at path \"" << path << "\"." << std::endl;
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

bool System::writeStringToFile(const std::string & str, const std::string & path){
	std::ofstream file(widen(path));
	if(file.bad() || file.fail()) {
		Log::Error() << "Unable to write to file at path \"" << path << "\"." << std::endl;
		return false;
	}
	file << str << std::endl;
	file.close();
	return true;
}

bool System::writeDataToFile(unsigned char * data, size_t size, const std::string & path) {
	std::ofstream outputFile(widen(path), std::ios::binary);

	if(!outputFile.is_open()) {
		Log::Error() << "Unable to save file at path \"" << path << "\"." << std::endl;
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

bool System::fileExists(const std::string& path){
	FILE* file = fopen( path.c_str(), "rb" );
	if(!file) {
		return false;
	}
	fclose(file);
	return true;
}
