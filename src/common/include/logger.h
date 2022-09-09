#ifndef COMMON_LOGGER_H
#define COMMON_LOGGER_H

#include <spdlog/spdlog.h>

namespace common {
	inline std::shared_ptr<spdlog::logger> graphics_logger() {
		return spdlog::get("graphics");
	}

	inline std::shared_ptr<spdlog::logger> field_logger() {
		return spdlog::get("field");
	}
}// namespace common

#endif //COMMON_LOGGER_H
