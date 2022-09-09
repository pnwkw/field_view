#include "glsl.h"

#include <logger.h>
#include <mag_cheb.h>

bool viz::glsl::checkSupport() noexcept {
	common::graphics_logger()->info("GLSL shader requirements satisfied by OpenGL core profile");
	return true;
}

viz::glsl::glsl() {
	gl::glCreateBuffers(1, &sol_segments_ssbo);
	if constexpr (common::isDebug) {
		gl::glObjectLabel(gl::GL_BUFFER, sol_segments_ssbo, -1, "sol_segments_ssbo");
	}

	gl::glCreateBuffers(1, &sol_params_ssbo);
	if constexpr (common::isDebug) {
		gl::glObjectLabel(gl::GL_BUFFER, sol_params_ssbo, -1, "sol_params_ssbo");
	}

	gl::glCreateBuffers(1, &dip_segments_ssbo);
	if constexpr (common::isDebug) {
		gl::glObjectLabel(gl::GL_BUFFER, dip_segments_ssbo, -1, "dip_segments_ssbo");
	}

	gl::glCreateBuffers(1, &dip_params_ssbo);
	if constexpr (common::isDebug) {
		gl::glObjectLabel(gl::GL_BUFFER, dip_params_ssbo, -1, "dip_params_ssbo");
	}

	mag = std::make_unique<mag_field::mag_cheb>();

	gl::glNamedBufferData(sol_segments_ssbo, sizeof(mag_field::mag_cheb::SolenoidSegmentsUniform), mag->getSolSegmentsPtr(), gl::GL_STATIC_COPY);
	gl::glNamedBufferData(sol_params_ssbo, sizeof(mag_field::mag_cheb::SolenoidParameterizationUniform), mag->getSolParamsPtr(), gl::GL_STATIC_COPY);

	gl::glNamedBufferData(dip_segments_ssbo, sizeof(mag_field::mag_cheb::DipoleSegmentsUniform), mag->getDipSegmentsPtr(), gl::GL_STATIC_COPY);
	gl::glNamedBufferData(dip_params_ssbo, sizeof(mag_field::mag_cheb::DipoleParameterizationUniform), mag->getDipParamsPtr(), gl::GL_STATIC_COPY);
}

viz::glsl::~glsl() {
	gl::glDeleteBuffers(1, &sol_segments_ssbo);
	gl::glDeleteBuffers(1, &sol_params_ssbo);
	gl::glDeleteBuffers(1, &dip_segments_ssbo);
	gl::glDeleteBuffers(1, &dip_params_ssbo);
}
