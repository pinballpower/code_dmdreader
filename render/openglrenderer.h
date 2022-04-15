#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../dmd/color.h"
#include "../dmd/palette.h"
#include "framerenderer.h"

#include "openglshader.h"

static const int tx_buf_len = 256 * 256 * 4;

class OpenGLRenderer : public FrameRenderer
{
public:


	OpenGLRenderer();
	~OpenGLRenderer();
	virtual void render_frame(DMDFrame& f);
	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer);

private:
	int width = 0;
	int height = 0;

	GLFWwindow* window = nullptr;
	OpenGLShader shader;
	unsigned int VAO, VBO, EBO;

	unsigned int dmd_texture_id = 0;
	unsigned int overlay_texture_id = 0;

	uint8_t texturbuf[tx_buf_len];

	float vertices[32] = {
		// positions          // texture coords
		 1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // top right
		 1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.12345f, // bottom right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.12345f, // bottom left
		-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 0.0f  // top left 
	};

	unsigned int indices[6] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	
	string overlay_texture_file;

	bool initialize_display();


};
