#ifndef COMMON_GLM_HELPER_H
#define COMMON_GLM_HELPER_H

#include <glm/glm.hpp>
#include <spdlog/fmt/ostr.h>
#include <unordered_set>

namespace common {
	struct hashIVec3 {
		size_t operator()(const glm::ivec3 &v) const {
			return std::hash<int>()(v.x) + std::hash<int>()(v.y) + std::hash<int>()(v.z);
		}
	};

	using unordered_ivec3_set = std::unordered_set<glm::ivec3, hashIVec3>;
}// namespace common

namespace glm {
	template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
	OStream &operator<<(OStream &os, const glm::vec<L, T, Q> &vec) {
		os << "[";

		for (glm::length_t i = 0; i < L - 1; ++i) {
			os << vec[i] << ", ";
		}

		os << vec[L - 1];

		os << "]";

		return os;
	}
}// namespace glm

#endif //COMMON_GLM_HELPER_H
