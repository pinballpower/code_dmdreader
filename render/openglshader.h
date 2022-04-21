#pragma once

#if defined(USE_OPENGL) || defined(USE_OPENGLES)

#include <string>

#ifdef USE_GLAD
#include <glad/glad.h>
#else
#include <GLES3/gl3.h>
#endif

#include <string>

using namespace std;

class OpenGLShader
{
public:
    unsigned int ID;

    OpenGLShader();
    OpenGLShader(const string vertexShader, const string fragmentShader);

    void compile_shaders(const string vertexShader, const string fragmentShader);


    // activate the shader
    // ------------------------------------------------------------------------
    void use();

    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const;

    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const;
    
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const;

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type); 
};

#endif