#ifndef VIZ_GLSL_H
#define VIZ_GLSL_H

#include <glbinding/gl46core/gl.h>
#include <glm/glm.hpp>
#include <glm_helper.h>

#include <config.h>
#include <context_base.h>
#include <mag_cheb.h>
#include <measurements.h>

namespace viz {
	class glsl {
	private:
		std::unique_ptr<mag_field::mag_cheb> mag;

		gl::GLuint sol_segments_ssbo{};
		gl::GLuint sol_params_ssbo{};
		gl::GLuint dip_segments_ssbo{};
		gl::GLuint dip_params_ssbo{};

	public:
		glsl();

		~glsl();

		gl::GLuint getSolSegmentsBufferName() { return sol_segments_ssbo; };

		gl::GLuint getDipSegmentsBufferName() { return dip_segments_ssbo; };

		gl::GLuint getSolParamsBufferName() { return sol_params_ssbo; };

		gl::GLuint getDipParamsBufferName() { return dip_params_ssbo; };

		static bool checkSupport() noexcept;
	};
}// namespace viz

#endif //VIZ_GLSL_H
