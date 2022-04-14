#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <boost/log/trivial.hpp>

#include "openglshader.h"
#include "openglrenderer.h"
#include "shader-definitions.h"


OpenGLRenderer::OpenGLRenderer()
{
}

OpenGLRenderer::~OpenGLRenderer()
{
}

void OpenGLRenderer::render_frame(DMDFrame& f)
{

	const vector<uint8_t> data = f.get_data();

	int tx_width, tx_height;
	int tx_pixel_count;
	if ((f.get_width() == 128) && f.get_height() == 32) {
		tx_width = tx_height = 128;
		tx_pixel_count = 128 * 32;
		vertices[7] = vertices[15] = 0.25f;

	}
	else if ((f.get_width() == 192) && f.get_height() == 64) {
		tx_width = tx_height = 192;
		tx_pixel_count = 192 * 64;
		vertices[15] = 0.33f;
		vertices[23] = 0.33f;
	}
	else {
		BOOST_LOG_TRIVIAL(warning) << "[openglrenderer] resolution " << f.get_width() << "x" << f.get_height() << "not supported";
		return;
	}

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);



	glBindTexture(GL_TEXTURE_2D, dmd_texture_id); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	if (f.get_bitsperpixel() == 24) {
		// copy data into texture buffer
		memcpy_s(texturbuf, tx_buf_len, &data[0], tx_pixel_count * 3);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tx_width, tx_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texturbuf);
	}
	else if (f.get_bitsperpixel() == 24) {
		// copy data into texture buffer
		memcpy_s(texturbuf, tx_buf_len, &data[0], tx_pixel_count * 4);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tx_width, tx_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,texturbuf);
	}

	// render
	// ------
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

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

	glfwSwapBuffers(window);
}

bool OpenGLRenderer::start_display()
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

	// build and compile our shader zprogram
	// ------------------------------------
	shader = OpenGLShader(vertexShader, fragmentShader192x64);

	BOOST_LOG_TRIVIAL(info) << "[openglrenderer] OpenGL version: " << glGetString(GL_VERSION);

	// texture 1
	// ---------
	glGenTextures(1, &dmd_texture_id);
	glBindTexture(GL_TEXTURE_2D, dmd_texture_id); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// No need for wrapping
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// This is
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	unsigned char* data = stbi_load("img/dmd_sample.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << "[openglrenderer] Failed to load texture1";
	}
	stbi_image_free(data);

	// texture 2
	// ---------
	glGenTextures(1, &overlay_texture_id);
	glBindTexture(GL_TEXTURE_2D, overlay_texture_id);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = stbi_load("img/circle_blurred.png", &width, &height, &nrChannels, 4);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		BOOST_LOG_TRIVIAL(error) << "[openglrenderer] Failed to load overlay_texture";
	}
	stbi_image_free(data);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	shader.use();


	return true;
}

bool OpenGLRenderer::configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer)
{
	width = 1920;
	height = 640;
	start_display();
	return true;
}
