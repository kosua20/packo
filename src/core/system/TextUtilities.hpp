#pragma once
#include "core/Common.hpp"

/**
 \brief Provides utilities process strings.
 */
class TextUtilities {

public:
	/** Trim characters from both ends of a string.
	 \param str the string to trim from
	 \param del the characters to delete
	 \return the trimmed string
	 */
	static std::string trim(const std::string & str, const std::string & del);

	/** Remove file extension from the end of a string.
	 \param str the string to remove the extension from
	 \return the extension string
	 */
	static std::string removeExtension(std::string & str);
	
	/** Return the parent directory of the current path is possible.
	 \param str the path to get the parent directory from.
	 \return the parent string
	 */
	static std::string parentDirectory(const std::string & str);

	static std::string fileName(const std::string & str);

	/** Replace all occurences of a substring in a string by another string.
	 \param source the string in which substitutions should happen
	 \param fromString substring to replace
	 \param toString new substring to insert
	 */
	static void replace(std::string & source, const std::string & fromString, const std::string & toString);

	/** Test if a string is a prefix of another string.
	 \param source the string to examine
	 \param prefix the prefix string to test
	 \return true if the prefix is here
	 */
	static bool hasPrefix(const std::string & source, const std::string & prefix);

	/** Test if a string is a suffix of another string.
	 \param source the string to examine
	 \param suffix the suffix string to test
	 \return true if the suffix is here
	 */
	static bool hasSuffix(const std::string & source, const std::string & suffix);
	
	/** Join a list of strings together using a custom delimiter.
	 \param tokens the list of strings to join
	 \param delimiter the string that will be inserted between each pair of strings
	 \return the result of the join
	 */
	static std::string join(const std::vector<std::string> & tokens, const std::string & delimiter);
	
	/** Split a string into a list of tokens based on a given delimiter.
	 \param str the string to split
	 \param delimiter the string that will be used as a splitting point
	 \param skipEmpty should empty tokens be ignored
	 \return a list of tokens
	 */
	static std::vector<std::string> split(const std::string & str, const std::string & delimiter, bool skipEmpty);
	
	static std::string lowercase(const std::string & src);

	static std::string padLeft(const std::string & s, size_t length, char c);
	
	static std::string padRight(const std::string & s, size_t length, char c);

	static bool isNumber(const std::string & s);

	static size_t count(const std::string & s);


};
