#include "core/system/Terminal.hpp"
#include "core/system/TextUtilities.hpp"

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include <iostream>

bool Terminal::_supportChecked = false;
bool Terminal::_supportANSI = false;

bool Terminal::supportsANSI(){
	if(!_supportChecked){
		_supportANSI = false;

#ifdef _WIN32
		// Enable color output.
		HANDLE h = GetStdHandle( STD_OUTPUT_HANDLE );
		if( h != INVALID_HANDLE_VALUE ) {
			DWORD dwMode = 0;
			if( GetConsoleMode( h, &dwMode ) ) {
				dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
				if( SetConsoleMode( h, dwMode ) ) {
					_supportANSI = true;
				}
			}
		}
		
#else
		// Check if we are in a terminal.
		char* termName = std::getenv("TERM");
		if(termName != nullptr){
			const std::string name(termName);
			const std::string nameLow = TextUtilities::lowercase(name);

			if(nameLow != "dumb"){
				// Check if we are redirected to a file.
				bool istty = isatty(fileno(stdout)) != 0;
				_supportANSI = istty;
			}
		}
#endif
		_supportChecked = true;
		
	}
	return _supportANSI;
}

void Terminal::disableANSI(){
	_supportANSI = false;
	_supportChecked = true;

}

void Terminal::outputUnicode( const std::string& str ) {
#ifdef _WIN32
	const int size = MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, nullptr, 0 );
	WCHAR* arr = new WCHAR[size];
	MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, static_cast<LPWSTR>( arr ), size );
	std::wstring res( arr );
	delete[] arr;
	std::wcout << res << std::flush;
#else
	std::cout << str << std::flush;
#endif
}

std::string Terminal::black(const std::string & s){
	return supportsANSI() ? ("\u001B[30m" + s + "\u001B[39m") : s;
}

std::string Terminal::red(const std::string & s){
	return supportsANSI() ? ("\u001B[31m" + s + "\u001B[39m") : s;
}

std::string Terminal::green(const std::string & s){
	return supportsANSI() ? ("\u001B[32m" + s + "\u001B[39m") : s;
}

std::string Terminal::yellow(const std::string & s){
	return supportsANSI() ? ("\u001B[33m" + s + "\u001B[39m") : s;
}

std::string Terminal::blue(const std::string & s){
	return supportsANSI() ? ("\u001B[34m" + s + "\u001B[39m") : s;
}

std::string Terminal::magenta(const std::string & s){
	return supportsANSI() ? ("\u001B[35m" + s + "\u001B[39m") : s;
}

std::string Terminal::cyan(const std::string & s){
	return supportsANSI() ? ("\u001B[36m" + s + "\u001B[39m") : s;
}

std::string Terminal::white(const std::string & s){
	return supportsANSI() ? ("\u001B[37m" + s + "\u001B[39m") : s;
}

std::string Terminal::bold(const std::string & s){
	return supportsANSI() ? ("\u001B[1m" + s + "\u001B[22m") : s;
}

std::string Terminal::dim(const std::string & s){
	return supportsANSI() ? ("\u001B[2m" + s + "\u001B[22m") : s;
}

std::string Terminal::italic(const std::string & s){
	return supportsANSI() ? ("\u001B[3m" + s + "\u001B[23m") : s;
}

std::string Terminal::underline(const std::string & s){
	return supportsANSI() ? ("\u001B[4m" + s + "\u001B[24m") : s;
}

std::string Terminal::inverse(const std::string & s){
	return supportsANSI() ? ("\u001B[7m" + s + "\u001B[27m") : s;
}

std::string Terminal::blackBg(const std::string & s){
	return supportsANSI() ? ("\u001B[40m" + s + "\u001B[49m") : s;
}

std::string Terminal::redBg(const std::string & s){
	return supportsANSI() ? ("\u001B[41m" + s + "\u001B[49m") : s;
}

std::string Terminal::greenBg(const std::string & s){
	return supportsANSI() ? ("\u001B[42m" + s + "\u001B[49m") : s;
}

std::string Terminal::yellowBg(const std::string & s){
	return supportsANSI() ? ("\u001B[43m" + s + "\u001B[49m") : s;
}

std::string Terminal::blueBg(const std::string & s){
	return supportsANSI() ? ("\u001B[44m" + s + "\u001B[49m") : s;
}

std::string Terminal::magentaBg(const std::string & s){
	return supportsANSI() ? ("\u001B[45m" + s + "\u001B[49m") : s;
}

std::string Terminal::cyanBg(const std::string & s){
	return supportsANSI() ? ("\u001B[46m" + s + "\u001B[49m") : s;
}

std::string Terminal::whiteBg(const std::string & s){
	return supportsANSI() ? ("\u001B[47m" + s + "\u001B[49m") : s;
}

std::string Terminal::brightBlack(const std::string & s){
	return supportsANSI() ? ("\u001B[90m" + s + "\u001B[39m") : s;
}

std::string Terminal::brightRed(const std::string & s){
	return supportsANSI() ? ("\u001B[91m" + s + "\u001B[39m") : s;
}

std::string Terminal::brightGreen(const std::string & s){
	return supportsANSI() ? ("\u001B[92m" + s + "\u001B[39m") : s;
}

std::string Terminal::brightYellow(const std::string & s){
	return supportsANSI() ? ("\u001B[93m" + s + "\u001B[39m") : s;
}

std::string Terminal::brightBlue(const std::string & s){
	return supportsANSI() ? ("\u001B[94m" + s + "\u001B[39m") : s;
}

std::string Terminal::brightMagenta(const std::string & s){
	return supportsANSI() ? ("\u001B[95m" + s + "\u001B[39m") : s;
}

std::string Terminal::brightCyan(const std::string & s){
	return supportsANSI() ? ("\u001B[96m" + s + "\u001B[39m") : s;
}

std::string Terminal::brightWhite(const std::string & s){
	return supportsANSI() ? ("\u001B[97m" + s + "\u001B[39m") : s;
}

std::string Terminal::brightBlackBg(const std::string & s){
	return supportsANSI() ? ("\u001B[100m" + s + "\u001B[49m") : s;
}

std::string Terminal::brightRedBg(const std::string & s){
	return supportsANSI() ? ("\u001B[101m" + s + "\u001B[49m") : s;
}

std::string Terminal::brightGreenBg(const std::string & s){
	return supportsANSI() ? ("\u001B[102m" + s + "\u001B[49m") : s;
}

std::string Terminal::brightYellowBg(const std::string & s){
	return supportsANSI() ? ("\u001B[103m" + s + "\u001B[49m") : s;
}

std::string Terminal::brightBlueBg(const std::string & s){
	return supportsANSI() ? ("\u001B[104m" + s + "\u001B[49m") : s;
}

std::string Terminal::brightMagentaBg(const std::string & s){
	return supportsANSI() ? ("\u001B[105m" + s + "\u001B[49m") : s;
}

std::string Terminal::brightCyanBg(const std::string & s){
	return supportsANSI() ? ("\u001B[106m" + s + "\u001B[49m") : s;
}

std::string Terminal::brightWhiteBg(const std::string & s){
	return supportsANSI() ? ("\u001B[107m" + s + "\u001B[49m") : s;
}
