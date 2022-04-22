#pragma once

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

    bool compile_shaders(const string vertexShader, const string fragmentShader);


    // activate the shader
    // ------------------------------------------------------------------------
    bool use();

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
    bool checkCompileErrors(unsigned int shader, std::string type); 

    bool shaders_ready = false;
};

