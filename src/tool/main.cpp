#include "core/Common.hpp"
#include "core/Strings.hpp"
#include "core/Graph.hpp"
#include "core/Evaluator.hpp"
#include "core/Random.hpp"

#include "core/system/Config.hpp"
#include "core/system/System.hpp"
#include "core/system/TextUtilities.hpp"
#include "core/system/Terminal.hpp"

#include <json/json.hpp>

class PackoConfig : public Config {
public:

	explicit PackoConfig(const std::vector<std::string> & argv) : Config(argv) {

		// Process arguments.
		for(const auto & arg : arguments()) {
			// Config path.
			if((arg.key == "in" || arg.key == "i") && !arg.values.empty()){
				inputDir = arg.values[0];
			}
			if((arg.key == "out" || arg.key == "o") && !arg.values.empty()){
				outputDir = arg.values[0];
			}
			if((arg.key == "graph" || arg.key == "g") && !arg.values.empty()){
				graphPath = arg.values[0];
			}
			
			if((arg.key == "resolution" || arg.key == "r") && arg.values.size() > 1){
				outResolution[0] = std::stoi(arg.values[0]);
				outResolution[1] = std::stoi(arg.values[1]);
				forceOutResolution = true;
			}
			if((arg.key == "seed" || arg.key == "s") && !arg.values.empty()){
				seed = std::stoi(arg.values[0]);
			}

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

		registerArgument("in", "i", "Input directory containing images to process.", "path to directory");
		registerArgument("out", "o", "Destination directory.", "path to directory");
		registerArgument("graph", "g", "Graph file.", "path to file");

		registerSection("Settings");
		registerArgument("resolution", "r", "Force the output resolution.", std::vector<std::string>{"w", "h"});
		registerArgument("seed", "s", "Integer seed for random number generation.", "seed");

		registerSection("Infos");
		registerArgument("version", "v", "Displays the current Packo version.");
		registerArgument("license", "", "Display the license message.");

	}

	fs::path inputDir;
	fs::path outputDir;
	fs::path graphPath;
	glm::ivec2 outResolution{64, 64};
	bool forceOutResolution = false;
	int seed = 743936;

	// Messages.
	bool version = false;
	bool license = false;
	bool bonus = false;
};

std::vector<fs::path> listFiles(const fs::path& dir){
	static const std::vector<std::string> validExts = {"png", "bmp", "tga", "jpeg"};
	if(!fs::exists(dir)){
		return {};
	}

	std::vector<fs::path> newPaths;
	// Don't recurse
	for (const fs::directory_entry& file : fs::directory_iterator(dir)) {
		const fs::path& path = file.path();
		std::string ext = path.extension().string();
		ext = TextUtilities::trim( ext, ".");
		if(std::find(validExts.begin(), validExts.end(), ext) != validExts.end()){
			newPaths.emplace_back(path);
		}
	}
	std::sort(newPaths.begin(), newPaths.end());
	return newPaths;
}

int main(int argc, char** argv){
	
	PackoConfig config(std::vector<std::string>(argv, argv+argc));
	if(config.version){
		Log::Info() << versionMessage << std::endl;
		return 0;
	} else if(config.license){
		Log::Info() << licenseMessage << std::endl;
		return 0;
	} else if(config.bonus){
		Log::Info() << bonusMessage << std::endl;
		return 0;
	} else if(config.showHelp(false)){
		return 0;
	}
	if(config.graphPath.empty() || config.outputDir.empty()){
		config.showHelp(true);
		return 0;
	}

	Random::seed(config.seed);

	// Load the graph.
	Graph graph;
	{
		const std::string path = config.graphPath.string();
		std::ifstream file(path);
		if(!file.is_open()){
			Log::Error() << "Unable to load graph from file at path \"" << path<< "\"" << std::endl;
			return 1;
		}

		json data = json::parse(file, nullptr, false);
		// Done with the file.
		file.close();
		// Invalid JSON.
		if(data.is_discarded()){
			Log::Error() << "Unable to parse graph from file at path \"" << path << "\"" << std::endl;
			return 1;
		}
		if(!graph.deserialize(data)){
			Log::Error() << "Unable to deserialize graph from file at path \"" << path << "\"" << std::endl;
			return 1;
		}
	}

	// List input files
	std::vector<fs::path> inputPaths;
	if(!config.inputDir.empty()){
		inputPaths = listFiles(config.inputDir);
		if(inputPaths.empty()){
			Log::Error() << "Unable to find input files while a directory was specified: path \"" << config.inputDir.string() << "\"" << std::endl;
			return 1;
		}
	}

	// Evaluate
	ErrorContext errorContext;
	bool res = evaluate(graph, errorContext, inputPaths, config.outputDir, config.outResolution, config.forceOutResolution);
	if(!res || errorContext.hasErrors()){
		Log::Error() << "Encountered an error while executing the graph." << std::endl;
		Log::Error() << errorContext.summarizeErrors() << std::endl;
		return 1;
	}

	return 0;
}
