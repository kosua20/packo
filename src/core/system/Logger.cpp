#include "core/system/Logger.hpp"
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#	include <io.h>
#	include <cstdio>
#else
#	include <unistd.h>
#endif

// We statically initialize the default logger.
// We don't really care about its exact construction/destruction moments,
// but we want it to always be created.
Log * Log::_defaultLogger = new Log();

void Log::set(Level l) {
	_level		  = l;
	_appendPrefix = true;
	if(_level == Level::VERBOSE && !_verbose) {
		// In this case, we want to ignore until the next flush.
		_ignoreUntilFlush = true;
		_appendPrefix	 = false;
	}
}

Log::Log() {

	// Check if the output is indeed a terminal, and not piped.
#ifdef _WIN32
	const bool isTerminal = _isatty(_fileno(stdout));
	char * env_p		  = nullptr;
	size_t size			  = 0;
	_dupenv_s(&env_p, &size, "TERM");
#else
	const bool isTerminal = isatty(fileno(stdout));
	char * env_p		  = std::getenv("TERM");
#endif
	if(isTerminal && env_p) {
		// Check if the output support colors.
		const std::vector<std::string> terms = {"xterm", "xterm-256", "xterm-256color", "vt100", "color", "ansi", "cygwin", "linux"};
		const std::string term(env_p);
		for(const auto & possibleTerm : terms) {
			if(term == possibleTerm) {
				_useColors = true;
				break;
			}
		}
	}
}

Log::Log(const std::string & filePath, bool logToStdin, bool verbose) :
	_logToStdOut(logToStdin), _verbose(verbose) {
	// Create file if it doesnt exist.
	setFile(filePath, false);
}

void Log::setFile(const std::string & filePath, bool flushExisting) {
	if(flushExisting) {
		_stream << std::endl;
		flush();
	}
	if(_file.is_open()) {
		_file.close();
	}
	_file.open(filePath, std::ofstream::app);
	if(_file.is_open()) {
		_file << "-- New session - " << time(nullptr) << " -------------------------------" << std::endl;
		_useColors = false;
	} else {
		std::cerr << "[Logger] Unable to create log file at path " << filePath << "." << std::endl;
	}
}

void Log::setVerbose(bool verbose) {
	_verbose = verbose;
}

void Log::setDefaultFile(const std::string & filePath) {
	_defaultLogger->setFile(filePath);
}

void Log::setDefaultVerbose(bool verbose) {
	_defaultLogger->setVerbose(verbose);
}

Log & Log::Info() {
	_defaultLogger->set(Level::INFO);
	return *_defaultLogger;
}

Log & Log::Warning() {
	_defaultLogger->set(Level::WARNING);
	return *_defaultLogger;
}

Log & Log::Error() {
	_defaultLogger->set(Level::ERROR);
	return *_defaultLogger;
}

Log & Log::Verbose() {
	_defaultLogger->set(Level::VERBOSE);
	return *_defaultLogger;
}

void Log::flush() {
	if(!_ignoreUntilFlush) {
		const std::string finalStr = _stream.str();

		if(_logToStdOut) {
			if(_level == Level::INFO || _level == Level::VERBOSE) {
				std::cout << finalStr << std::flush;
			} else {
				std::cerr << finalStr << std::flush;
			}
		}
		if(_file.is_open()) {
			_file << finalStr << std::flush;
		}
	}
	_ignoreUntilFlush = false;
	_appendPrefix	 = false;
	_stream.str(std::string());
	_stream.clear();
	_level = Level::INFO;
}

void Log::appendIfNeeded() {
	if(_appendPrefix) {
		_appendPrefix = false;
		if(_useColors) {
			_stream << _colorStrings[int(_level)];
		}
		_stream << _levelStrings[int(_level)];
	}
}

Log & Log::operator<<(const Domain & domain) {

	if(_appendPrefix && _useColors) {
		_stream << _colorStrings[int(_level)];
	}
	_stream << "[" << _domainStrings[domain] << "] ";

	if(_appendPrefix) {
		_stream << _levelStrings[int(_level)];
		_appendPrefix = false;
	}
	return *this;
}

Log & Log::operator<<(std::ostream & (*modif)(std::ostream &)) {
	appendIfNeeded();

	modif(_stream);

	flush();
	return *this;
}

Log & Log::operator<<(std::ios_base & (*modif)(std::ios_base &)) {
	modif(_stream);
	return *this;
}
