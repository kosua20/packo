#pragma once

#include "core/Common.hpp"
#include <random>
#include <mutex>

class Random {
public:
	/** Seed the shared generator using a random number.
	 \note The seed is obtained through a std::random_device.
	 \warning Threads created before the call won't be seeded (except for the calling thread).
	 \note It is recommended to seed the generator on the main thread at the beginning of the application execution.
	 */
	static void seed();

	/** Seed the shared generator using a given number.
	 \param seedValue the seed to use
	 \warning Threads created before the call won't be seeded (except for the calling thread). 
	 \note It is recommended to seed the generator on the main thread at the beginning of the application execution.
	 */
	static void seed(unsigned int seedValue);

	/** Query the current global seed.
	 \return the current global seed
	 */
	static unsigned int getSeed();

	/** Generate an integer in a given interval.
	 \param min the included lower bound
	 \param max the included higher bound
	 \return an integer in [min, max]
	 */
	static int Int(int min, int max);

	/** Generate a float in [0.0, 1.0)
	 \return a float in [0.0, 1.0)
	 */
	static float Float();

	/** Generate a float in a given interval.
	 \param min the included lower bound
	 \param max the excluded higher bound
	 \return a float in [min, max)
	 */
	static float Float(float min, float max);

	/** Generate a random color in RGB color space.
	 Internally a HSV color with reasonable saturation and luminance
	 is generated and converted to RGB.
	 \return an RGBA.
	 */
	static glm::vec4 Color();

private:
	/** \brief A MT19937 generator seeded using the shared generator.
	 	Used to provide per-thread MT19937 generators in a thread-safe way.
	 */
	struct LocalMT19937 {

		/** Constructor. */
		LocalMT19937();

		std::mt19937 mt;	   ///< The randomness generator.
		unsigned int seed = 0; ///<The local seed.
	};

	static unsigned int _seed;				  ///< The current main seed.
	static std::mt19937 _shared;			  ///< Shared randomness generator, used for seeding per-thread generators. \warning Not thread safe.
	static std::mutex _lock;				  ///< The lock for the shared generator.
	static thread_local LocalMT19937 _thread; ///< Per-thread randomness generator, seeded using the shared generator.
};

