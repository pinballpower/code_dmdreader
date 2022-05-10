#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "openglrenderer.hpp"

class GladOpenGLRenderer : public OpenGLRenderer
{

public:
	GladOpenGLRenderer();

private:
	GLFWwindow* window = nullptr;
	virtual bool initializeDisplay() override;
	virtual void swapBuffers() override;

};
