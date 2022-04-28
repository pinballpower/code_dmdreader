#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "openglrenderer.h"

class Pi4Renderer : public OpenGLRenderer
{
public:
	Pi4Renderer();
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer) override;

private:
	virtual bool initializeDisplay() override;
	virtual void swapBuffers() override;

	int displayNumber = 0;

};
