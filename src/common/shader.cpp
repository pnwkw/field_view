#include "shader.h"

#include <config.h>
#include <stdexcept>
#include <logger.h>

void checkShaderStatus(gl::GLuint shader, const std::string &shaderFilename) {
    gl::GLint error_status;

    gl::glGetShaderiv(shader, gl::GL_COMPILE_STATUS, &error_status);
    if (!error_status) {
        common::graphics_logger()->error("Error compiling shader {}", shaderFilename);

        gl::GLint maxLength = 0;
        gl::glGetShaderiv(shader, gl::GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<gl::GLchar> infoLog(maxLength);
        gl::glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

        common::graphics_logger()->error(infoLog.data());

        gl::glDeleteShader(shader);

        throw std::runtime_error(infoLog.data());
    }
}

gl::GLuint common::loadCompileShaderString(gl::GLenum shaderType, const std::vector<const gl::GLchar*> &shaderTexts, const std::string &name, const Specialization *spec) {
    gl::GLuint shader = gl::glCreateShader(shaderType);
    if constexpr (common::isDebug) {
        gl::glObjectLabel(gl::GL_SHADER, shader, -1, name.c_str());
    }

    std::vector<gl::GLint> shaderLengths;

    gl::glShaderSource(shader, shaderTexts.size(), shaderTexts.data(), shaderLengths.data());

    gl::glCompileShader(shader);

    checkShaderStatus(shader, name);

    return shader;
}

gl::GLuint common::createProgram(const std::initializer_list<gl::GLuint> &shaderVector, const char *feedback) {
	gl::GLuint program = gl::glCreateProgram();

	for (const auto shaderObj : shaderVector) {
		gl::glAttachShader(program, shaderObj);
	}

	if (feedback) {
		const char *ptr = feedback;
		gl::glTransformFeedbackVaryings(program, 1, &ptr, gl::GL_INTERLEAVED_ATTRIBS);
	}

	gl::glLinkProgram(program);

	gl::GLint error_status;

	gl::glGetProgramiv(program, gl::GL_LINK_STATUS, &error_status);

	if (!error_status) {
		common::graphics_logger()->error("Error linking program");

		gl::GLint maxLength = 0;
		gl::glGetProgramiv(program, gl::GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<gl::GLchar> infoLog(maxLength);
		gl::glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());

		common::graphics_logger()->error(infoLog.data());

		gl::glDeleteProgram(program);

		throw std::runtime_error("Error linking program");
	}

	return program;
}
