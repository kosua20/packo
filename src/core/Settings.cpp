#include "core/Settings.hpp"
#include "core/system/TextUtilities.hpp"

PackoConfig::PackoConfig(const std::vector<std::string> & argv) : Config(argv) {

	// Process arguments.
	for(const auto & arg : arguments()) {
		// Config path.

		if(arg.key == "version" || arg.key == "v") {
			version = true;
		}
		if(arg.key == "license") {
			license = true;
		}
		if(arg.key == "coucou") {
			bonus = true;
		}
	}

	registerSection("Settings");

	registerSection("Infos");
	registerArgument("version", "v", "Displays the current Calco version.");
	registerArgument("license", "", "Display the license message.");

}
