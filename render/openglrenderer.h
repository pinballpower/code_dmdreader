#pragma once

#define GLFW_INCLUDE_NONE
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
	virtual void renderFrame(DMDFrame& f) override;
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer) override;

protected:

	virtual void swapBuffers();
	virtual bool initializeDisplay();

	int width = 0;
	int height = 0;
	bool scale_linear = false;

	int frame_width = 0, frame_height = 0;

	// DMD texture data
	int tx_width = 0, tx_height = 0;
	int tx_pixel_count = 0;

	// DMD position
	int dmd_x = 0, dmd_y = 0, dmd_width = 0, dmd_height = 0;

	OpenGLShader shader;
	unsigned int VAO = 0, VBO = 0, EBO = 0;

	unsigned int dmd_texture_id = 0;
	unsigned int overlay_texture_id = 0;

	/// <summary>
	/// disables swap_buffers, only rendering without displaying 
	/// </summary>
	bool no_display = false;

	uint8_t texturbuf[tx_buf_len];

	float vertices[20] = {
		// positions          // texture coords
		 1.0f,  1.0f, 0.0f,   1.0f, 0.0f, // top right
		 1.0f, -1.0f, 0.0f,   1.0f, 0.12345f, // bottom right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.12345f, // bottom left
		-1.0f,  1.0f, 0.0f,   0.0f, 0.0f  // top left 
	};

	unsigned int indices[6] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	string overlay_texture_file;

	string shader_prefix = "";
	string vertex_shader;
	string fragment_shader;

	void recalcVertices();
	void initializeOpenGL();
	void loadShaders(string vs, string fs);

};