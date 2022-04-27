#pragma once

#include "openglrenderer.h"

class GladOpenGLRenderer : public OpenGLRenderer
{

public:
	GladOpenGLRenderer();

private:
	GLFWwindow* window = nullptr;
	virtual bool initializeDisplay() override;
	virtual void swapBuffers() override;

};
