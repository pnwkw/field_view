#ifndef COMMON_SHADER_H
#define COMMON_SHADER_H

#include <cstdint>
#include <string>
#include <vector>

#include <glbinding/gl46core/gl.h>

namespace common {
	typedef struct {
		std::vector<gl::GLuint> indexes;
		std::vector<gl::GLuint> values;
	} Specialization;

    gl::GLuint loadCompileShaderString(gl::GLenum shaderType, const std::vector<const gl::GLchar*> &shaderTexts, const std::string &name, const Specialization *spec = nullptr);

	gl::GLuint createProgram(const std::initializer_list<gl::GLuint> &shaderVector, const char *feedback = nullptr);
}// namespace common

#endif //COMMON_SHADER_H
