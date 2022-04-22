#pragma once

#include "openglrenderer.h"

class GladOpenGLRenderer : public OpenGLRenderer
{

public:
	GladOpenGLRenderer();

private:
	GLFWwindow* window = nullptr;
	virtual bool initialize_display();
	virtual void swap_buffers();

};
