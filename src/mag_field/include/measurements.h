#ifndef MAG_FIELD_MEASUREMENTS_H
#define MAG_FIELD_MEASUREMENTS_H

#include <config.h>
#include <cstdint>
#include <glm/glm.hpp>

namespace mag_field {
	constexpr std::int32_t x_detector = 512;
	constexpr std::int32_t y_detector = 512;
	constexpr std::int32_t z_detector = 1312;

	constexpr int PipeZMin = 570;
	constexpr int PipeZMax = 850;

	constexpr glm::ivec3 center{0, 0, -455};
	constexpr glm::ivec3 detector_dimensions{x_detector * 2, y_detector * 2, z_detector * 2};

	constexpr glm::ivec3 detector_min{center.x - x_detector, center.y - y_detector, center.z - z_detector};
	constexpr glm::ivec3 detector_max{center.x + x_detector, center.y + y_detector, center.z + z_detector};

	constexpr int SolR = 480;
	constexpr int SolZMin = -530;
	constexpr int SolZMax = 550;

}// namespace mag_field

#endif //MAG_FIELD_MEASUREMENTS_H
