#include "config.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace common {
	std::size_t deviceID;
	std::size_t randomSeed;

	void configRead(const std::string &name) {
		std::ifstream i(name);
		nlohmann::json j;
		i >> j;

		if (j.find("deviceID") != j.end()) {
			deviceID = j["deviceID"];
		} else {
			deviceID = 0;
		}

		if (j.find("randomSeed") != j.end()) {
			randomSeed = j["randomSeed"];
		} else {
			randomSeed = 0xC0FFE;
		}
	}
}// namespace common
