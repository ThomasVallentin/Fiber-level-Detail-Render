#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include "FilePath.hpp"
#include <glad/glad.h>

#define GLIMAC_SHADER_SRC(str) #str

namespace glimac {

class Shader {
public:
	Shader(GLenum type): m_nGLId(glCreateShader(type)) {
	}

	~Shader() {
		glDeleteShader(m_nGLId);
	}

	Shader(Shader&& rvalue): m_nGLId(rvalue.m_nGLId) {
		rvalue.m_nGLId = 0;
	}

	Shader& operator =(Shader&& rvalue) {
		m_nGLId = rvalue.m_nGLId;
		rvalue.m_nGLId = 0;
		return *this;
	}

	GLuint getGLId() const {
		return m_nGLId;
	}

	void setSource(const char* src) {
		glShaderSource(m_nGLId, 1, &src, 0);
	}

	bool compile();

	const std::string getInfoLog() const;

private:
	Shader(const Shader&);
	Shader& operator =(const Shader&);

	GLuint m_nGLId;
};

// Load a shader (but does not compile it)
Shader loadShader(GLenum type, const FilePath& filepath);

}
