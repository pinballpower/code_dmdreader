#include <filesystem> 

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <boost/log/trivial.hpp>

#include "openglshader.h"
#include "openglrenderer.h"

OpenGLRenderer::OpenGLRenderer()
{
}

OpenGLRenderer::~OpenGLRenderer()
{
}

void OpenGLRenderer::recalcVertices() {

	//vertices = {
	//	// positions          // texture coords
	//	 1.0f,  1.0f, 0.0f,   1.0f, 0.0f, // top right
	//	 1.0f, -1.0f, 0.0f,   1.0f, 0.12345f, // bottom right
	//	-1.0f, -1.0f, 0.0f,   0.0f, 0.12345f, // bottom left
	//	-1.0f,  1.0f, 0.0f,   0.0f, 0.0f  // top left 
	//};

	float left, right, top, bottom, dmd_scale_y = 0;

	top = .8;
	right = 1;
	bottom = -1;
	left = -1;

	top = 1.0f - 2.0f*((float)dmd_y / (float)height);
	left = -1.0f + 2.0f*((float)dmd_x / (float)width);
	bottom = -1.0f + 2.0f*((float)(height - dmd_y - dmd_height) / height);
	right = 1.0f - 2.0f*((float)(width - dmd_x - dmd_width) / width);

	vertices[0] = vertices[5] = right;
	vertices[10] = vertices[15] = left;
	vertices[1] = vertices[16] = top;
	vertices[6] = vertices[11] = bottom;
}

void OpenGLRenderer::initializeOpenGL()
{
	BOOST_LOG_TRIVIAL(info) << "[openglrenderer] OpenGL version: " << glGetString(GL_VERSION);

	// texture 1  - the DMD dislay
	// ---------------------------
	glGenTextures(1, &dmd_texture_id);
	glBindTexture(GL_TEXTURE_2D, dmd_texture_id); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

	// texture scaling
	if (scale_linear) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}


	// texture 2 - the circle overlay
	// ------------------------------
	glGenTextures(1, &overlay_texture_id);
	glBindTexture(GL_TEXTURE_2D, overlay_texture_id);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nrChannels;
	unsigned char* data = stbi_load(overlay_texture_file.c_str(), &width, &height, &nrChannels, 4);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		BOOST_LOG_TRIVIAL(debug) << "[openglrenderer] loaded overlay_texture " << overlay_texture_file;
		stbi_image_free(data);
	}
	else
	{
		BOOST_LOG_TRIVIAL(warning) << "[openglrenderer] Failed to load overlay_texture " << overlay_texture_file << ", will use no overlay";
		data = new unsigned char[4]{ 0xff, 0xff, 0xff, 0xff }; // create a one-pixel texture that's completely transparent
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		delete[] data;
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	shader.use();

}

void OpenGLRenderer::renderFrame(DMDFrame& f)
{

	const vector<uint8_t> data = f.get_data();

	// resolution changed, this usually only happens rendering the first frame 
	if ((f.get_width() != frame_width) || (f.get_height() != frame_height)) {
		frame_width = f.get_width();
		frame_height = f.get_height();
		if ((f.get_width() == 128) && f.get_height() == 32) {
			tx_width = tx_height = 128;
			tx_pixel_count = 128 * 32;
			vertices[9] = vertices[14] = 0.25f;
		}
		else if ((f.get_width() == 192) && f.get_height() == 64) {
			tx_width = tx_height = 192;
			tx_pixel_count = 192 * 64;
			vertices[9] = vertices[14] = 0.3333f;
		}
		else {
			BOOST_LOG_TRIVIAL(warning) << "[openglrenderer] resolution " << f.get_width() << "x" << f.get_height() << "not supported";
			return;
		}
	}

	// start drawing
	// -------------
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


	// render DMD
	// ----------

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// range checking
	assert(tx_buf_len >= tx_pixel_count * 4);
	if (tx_buf_len < tx_pixel_count * 4) {
		BOOST_LOG_TRIVIAL(error) << "[openglrenderer] buffer too small, aborting";
		return;
	}

	glBindTexture(GL_TEXTURE_2D, dmd_texture_id); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	if (f.get_bitsperpixel() == 24) {
		// copy data into texture buffer
		memcpy(texturbuf, &data[0], tx_pixel_count * 3);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tx_width, tx_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texturbuf);
	}
	else if (f.get_bitsperpixel() == 24) {
		// copy data into texture buffer
		memcpy(texturbuf, &data[0], tx_pixel_count * 4);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tx_width, tx_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texturbuf);
	}

	// bind Textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dmd_texture_id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, overlay_texture_id);

	// Set textures
	glUniform1i(glGetUniformLocation(shader.ID, "texture1"), 0);
	glUniform1i(glGetUniformLocation(shader.ID, "texture2"), 1);

	// render container
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);




	if (!no_display) {
		this->swapBuffers();
	}
}

bool OpenGLRenderer::initializeDisplay()
{
	BOOST_LOG_TRIVIAL(error) << "[openglrenderer] initialize_display not implemented";
	return false;
}

bool OpenGLRenderer::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer)
{
	// A renderer might ignore these if the resolution is fixed
	width = pt_renderer.get("width", 1280);
	height = pt_renderer.get("height", 320);

	scale_linear = pt_renderer.get("scale_linear", false);

	dmd_x = pt_renderer.get("dmd_x", 0);
	dmd_y = pt_renderer.get("dmd_y", 0);
	dmd_width = pt_renderer.get("dmd_width", 0);
	dmd_height = pt_renderer.get("dmd_height", 0);

	if (dmd_width == 0) {
		dmd_width = width;
	}

	if (dmd_height == 0) {
		dmd_height = height;
	}

	overlay_texture_file = pt_renderer.get("overlay_texture", "img/circle_blurred.png");

	fragment_shader = pt_renderer.get("fragment_shader", "shaders/"+shader_prefix + "-128x32.fs");
	vertex_shader = pt_renderer.get("vertex_shader", "shaders/" + shader_prefix + ".vs");

	// allow to turn of display, e.g. to test the performance
	no_display = ! pt_renderer.get("display", true);

	initializeDisplay();
	initializeOpenGL();

	// width and height might have been changed here - depnding on the renderer (e.g. if the renderer is a fullscreen renderer)

	loadShaders(vertex_shader, fragment_shader);
	recalcVertices();

	return true;
}

void OpenGLRenderer::swapBuffers()
{
	BOOST_LOG_TRIVIAL(error) << "[openglrenderer] swapBuffers not implemented";
}

void OpenGLRenderer::loadShaders(string vs, string fs) {
	string vs_code, fs_code;
	if (std::filesystem::exists(vs)) {
		std::ifstream vs_file(vs);
		std::stringstream buffer;
		buffer << vs_file.rdbuf();
		vs_code = buffer.str();
		BOOST_LOG_TRIVIAL(debug) << "[openglrenderer] loaded vertex shader";
	}
	else {
		vs_code = vs;
		BOOST_LOG_TRIVIAL(info) << "[openglrenderer] vertex shader is not a file, trying to use it directly";

	}

	if (std::filesystem::exists(fs)) {
		std::ifstream fs_file(fs);
		std::stringstream buffer;
		buffer << fs_file.rdbuf();
		fs_code = buffer.str();
		BOOST_LOG_TRIVIAL(debug) << "[openglrenderer] loaded fragment shader";
	}
	else {
		fs_code = fs;
		BOOST_LOG_TRIVIAL(info) << "[openglrenderer] fragment shader is not a file, trying to use it directly";
	}

	shader = OpenGLShader();
	if (!shader.compileShaders(vs_code, fs_code)) {
		BOOST_LOG_TRIVIAL(error) << "[openglrenderer] couldn't compile OpenGL shaders, output might be incorrect";
	}
	else {
		BOOST_LOG_TRIVIAL(info) << "[openglrenderer] sucessfully compiled shaders";
	}
	shader.use();
	
}

