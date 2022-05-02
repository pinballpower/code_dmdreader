#include <boost/log/trivial.hpp>

#include "openglshader.h"
#include "../util/image.h"

OpenGLShader::OpenGLShader() {

}

OpenGLShader::OpenGLShader(const string vertexShader, const string fragmentShader)
{
	compileShaders(vertexShader, fragmentShader);
}

bool OpenGLShader::compileShaders(const string vertexShader, const string fragmentShader) {
	shaders_ready = true;

	const char* vShaderCode = vertexShader.c_str();
	const char* fShaderCode = fragmentShader.c_str();
	// 2. compile shaders
	unsigned int vertex, fragment;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	if (!checkCompileErrors(vertex, "vertex")) {
		shaders_ready = false;
	}
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	if (!checkCompileErrors(fragment, "fragment")) {
		shaders_ready = true;
	}
	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	if (!checkCompileErrors(ID, "program")) {
		shaders_ready = true;
	}
	// delete the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	return shaders_ready;
}


// activate the shader
// ------------------------------------------------------------------------
bool OpenGLShader::use()
{
	glUseProgram(ID);
	return shaders_ready;
}
// utility uniform functions
// ------------------------------------------------------------------------
void OpenGLShader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void OpenGLShader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void OpenGLShader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
bool OpenGLShader::checkCompileErrors(unsigned int shader, std::string type)
{
	int success;
	char infoLog[1024];
	if (type != "program")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			BOOST_LOG_TRIVIAL(error) << "[shader] compilation error of type " << type;
			return false;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			BOOST_LOG_TRIVIAL(error) << "[shader] linking error of type " << type;
			return false;
		}
	}
	return true;
}
