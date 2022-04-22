#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "openglrenderer.h"

class Pi4Renderer : public OpenGLRenderer
{
private:
	virtual bool initialize_display();
	virtual void swap_buffers();
};
