#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "openglrenderer.h"

class Pi4Renderer : public OpenGLRenderer
{
public:
	Pi4Renderer();

private:
	virtual bool initializeDisplay() override;
	virtual void swapBuffers() override;
};
