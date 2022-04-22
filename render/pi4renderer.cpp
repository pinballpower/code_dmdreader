#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>

#include "ogl-pi4.h"
#include "pi4renderer.h"


void Pi4Renderer::swap_buffers() {
	gbmSwapBuffers();
}

bool Pi4Renderer::initialize_display()
{
	start_fullscreen_ogl();
	return true;
}
