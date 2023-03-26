#pragma once

#include "core/system/Config.hpp"
#include "core/Common.hpp"


/**
 \brief Terminal helpers
 \ingroup System
 */
class Terminal {
public:

	static bool supportsANSI();

	static void disableANSI();

	static void outputUnicode( const std::string& str );

	static std::string black(const std::string & s);

	static std::string red(const std::string & s);

	static std::string green(const std::string & s);

	static std::string yellow(const std::string & s);

	static std::string blue(const std::string & s);

	static std::string magenta(const std::string & s);

	static std::string cyan(const std::string & s);

	static std::string white(const std::string & s);

	static std::string bold(const std::string & s);

	static std::string dim(const std::string & s);

	static std::string italic(const std::string & s);

	static std::string underline(const std::string & s);

	static std::string inverse(const std::string & s);

	static std::string blackBg(const std::string & s);

	static std::string redBg(const std::string & s);

	static std::string greenBg(const std::string & s);

	static std::string yellowBg(const std::string & s);

	static std::string blueBg(const std::string & s);

	static std::string magentaBg(const std::string & s);

	static std::string cyanBg(const std::string & s);

	static std::string whiteBg(const std::string & s);

	static std::string brightBlack(const std::string & s);

	static std::string brightRed(const std::string & s);

	static std::string brightGreen(const std::string & s);

	static std::string brightYellow(const std::string & s);

	static std::string brightBlue(const std::string & s);

	static std::string brightMagenta(const std::string & s);

	static std::string brightCyan(const std::string & s);

	static std::string brightWhite(const std::string & s);

	static std::string brightBlackBg(const std::string & s);

	static std::string brightRedBg(const std::string & s);

	static std::string brightGreenBg(const std::string & s);

	static std::string brightYellowBg(const std::string & s);

	static std::string brightBlueBg(const std::string & s);

	static std::string brightMagentaBg(const std::string & s);

	static std::string brightCyanBg(const std::string & s);

	static std::string brightWhiteBg(const std::string & s);

private:

	static bool _supportChecked;
	static bool _supportANSI;
};
