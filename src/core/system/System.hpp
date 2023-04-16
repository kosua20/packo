#pragma once

#include "core/system/Config.hpp"
#include "core/Common.hpp"

#define GHC_FILESYSTEM_FWD
#include <ghc/filesystem.hpp>

namespace fs = ghc::filesystem;

#include <thread>

/**
 \brief Performs system basic operations such as directory creation, timing, threading, file picking.
 \ingroup System
 */
class System {
public:

	/** Notify the user by sending a 'Bell' signal. */
	static void ping();

	static std::string loadStringFromFile(const fs::path& path);
	
	static bool writeStringToFile(const std::string & str, const fs::path& path);

	static bool writeDataToFile(unsigned char * data, size_t size, const fs::path& path);

	static std::string timestamp();

	template<typename ThreadFunc>
	static void forParallel(size_t low, size_t high, ThreadFunc func) {
		// Make sure the loop is increasing.
		if(high < low) {
			const size_t temp = low;
			low				  = high;
			high			  = temp;
		}
		// Prepare the threads pool.
		// Always leave one thread free.
		const size_t count = size_t(std::max(int(std::thread::hardware_concurrency())-1, 1));
		std::vector<std::thread> threads;
		threads.reserve(count);

		// Compute the span of each thread.
		const size_t span = std::max(size_t(1), (high - low) / count);
		// Helper to execute the function passed on a subset of the total interval.
		auto launchThread = [&func](size_t a, size_t b) {
			for(size_t i = a; i < b; ++i) {
				func(i);
			}
		};

		for(size_t tid = 0; tid < count; ++tid) {
			// For each thread, call the same lambda with different bounds as arguments.
			const size_t threadLow  = tid * span;
			const size_t threadHigh = tid == (count-1) ? high : ((tid + 1) * span);
			threads.emplace_back(launchThread, threadLow, threadHigh);
		}
		// Wait for all threads to finish.
		std::for_each(threads.begin(), threads.end(), [](std::thread & x) { x.join(); });
	}
};
