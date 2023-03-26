#pragma once

#include "core/Common.hpp"
#include <vector>

/** \brief Represent a key-values tuple.
  	\ingroup System
 */
struct KeyValues {

	std::string key;				 ///< The key.
	std::vector<std::string> values; ///< A vector of values.
	std::vector<KeyValues> elements; ///< A vector of child elements.

	/** Constructor from a key.
	 \param aKey the key of the new tuple
	 */
	explicit KeyValues(const std::string & aKey);
};

/**
 \brief Contains configurable elements as attributes, populated from the command line, a configuration file or default values.
 \ingroup System
 */
class Config {
public:
	/** Initialize a new config object, parsing the input arguments and filling the attributes with their values.
	 	\param argv the raw input arguments
	 */
	explicit Config(const std::vector<std::string> & argv);

	/** Display help using the logger if the '--help' argument has been passed.
	 \param force display even if --help wasn't passed.
	 \return true if the help is shown, can be used for early exit
	 */
	bool showHelp(bool force);

protected:
	/** \brief Informations about an argument. */
	struct ArgumentInfo {

		/** Constructor.
		 \param aname argument long name
		 \param ashort argument optional short name
		 \param adetails argument description
		 \param avalues optional list of parameters for the argument
		 */
		ArgumentInfo(const std::string & aname, const std::string & ashort, const std::string & adetails,
			const std::vector<std::string> & avalues);

		std::string nameLong;			 ///< The main argument name.
		std::string nameShort;			 ///< The short argument name.
		std::string details;			 ///< Argument description.
		std::vector<std::string> values; ///< Zero, one or multiple argument parameters.
	};

	/** List of raw arguments.
	 \return the arguments list
	 */
	const std::vector<KeyValues> & arguments() const;

	/** Create a section in the help messages, to group arguments.
	 \param name the section name
	 */
	void registerSection(const std::string & name);
	
	/** Register help infos for an argument. The command will be listed as follow in the help message:
	 "--shortName,--longName <param0> <param1>	Details."
	 \param longName the command full name
	 \param shortName the command abbreviated name
	 \param details the command description
	 \param params name or type of each expected parameter
	 */
	void registerArgument(const std::string & longName, const std::string & shortName, const std::string & details, const std::vector<std::string> & params = {});
	
	/** Register help infos for an argument. The command will be listed as follow in the help message:
	 "--shortName,--longName <param>	Details."
	 \param longName the command full name
	 \param shortName the command abbreviated name
	 \param details the command description
	 \param param name or type of the expected parameter
	 */
	void registerArgument(const std::string & longName, const std::string & shortName, const std::string & details, const std::string & param);

private:
	/**
	 Helper to extract (key, [values]) from a configuration file on disk.
	 \param filePath the path to the configuration file.
	 \param arguments a vector will be populated with (key, [values]) tuples.
	 */
	static void parseFromFile(const std::string & filePath, std::vector<KeyValues> & arguments);

	/**
	 Helper to extract (key, [values]) from the given command-line arguments.
	 \param argv the raw input arguments
	 \param arguments  a vector will be populated with (key, [values]) tuples.
	 */
	static void parseFromArgs(const std::vector<std::string> & argv, std::vector<KeyValues> & arguments);

	/// Store the internal parsed (keys, [values]) extracted from a file or the command-line.
	std::vector<KeyValues> _rawArguments;

	/// Store informations about each argument, for displaying the help message.
	std::vector<ArgumentInfo> _infos;

	bool _showHelp = false; ///< Should the help be displayed.
};
