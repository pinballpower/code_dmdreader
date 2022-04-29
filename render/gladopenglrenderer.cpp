#include <boost/log/trivial.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "gladopenglrenderer.h"

GladOpenGLRenderer::GladOpenGLRenderer() {
	shader_prefix = "ogl";
}

bool GladOpenGLRenderer::initializeDisplay()
{
	// glfw: initialize and configure
// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	window = glfwCreateWindow(width, height, "DMD", NULL, NULL);
	if (window == nullptr)
	{
		BOOST_LOG_TRIVIAL(error) << "[openglrenderer] failed to create GLFW window";
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);


	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		BOOST_LOG_TRIVIAL(error) << "[openglrenderer] failed to initialize GLAD";
		return false;
	}



	return true;
}

void GladOpenGLRenderer::swapBuffers()
{
	glfwSwapBuffers(window);
}
