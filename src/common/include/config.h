#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

#include <cstddef>
#include <string>

namespace common {
	constexpr std::size_t windowWidth = 1280;
	constexpr std::size_t windowHeight = 720;
	constexpr bool headless = false;

	extern std::size_t randomSeed;

	constexpr std::size_t samples = 3000;
	extern std::size_t deviceID;

	void configRead(const std::string &name);

#ifdef NDEBUG
	constexpr bool isDebug = false;
#else
	constexpr bool isDebug = true;
#endif
}// namespace common

#endif //COMMON_CONFIG_H
