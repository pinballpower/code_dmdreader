#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"


#include "image.h"

RGBBuffer::RGBBuffer()
{
	width = height = 0;
}

RGBBuffer::RGBBuffer(int width, int height, bool alpha)
{
	assert((width >= 0) && (height >= 0));

	this->width = width;
	this->height = height;
	this->alpha = alpha;
	if (alpha) {
		bytesPerPixel = 4;
	} else{
		bytesPerPixel = 3;
	}

	// initialize an empty picture
	data = vector<uint8_t>(width * height * bytesPerPixel);
}

RGBBuffer::~RGBBuffer()
{
}

const vector<uint8_t> &RGBBuffer::getData() const
{
	return data;
}

void RGBBuffer::setPixel(int x, int y, rgb rgbdata)
{
	int offset = (x + y * width) * bytesPerPixel;
	data[offset] = rgbdata.r;
	data[offset+1] = rgbdata.g;
	data[offset+2] = rgbdata.b;
}

void RGBBuffer::setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	int offset = (x + y * width) * bytesPerPixel;
	data[offset] = r;
	data[offset+1] = g;
	data[offset+2] = b;
	if (this->alpha) {
		data[offset + 3] = a;
	}
}

RGBBuffer RGBBuffer::fromPNG(const string filename, bool useAlpha)
{
	int width = 0;
	int height = 0;
	int bpp = 0;
	unsigned char* imageData;
	int bufflen;
	
	if (useAlpha) {
		imageData = stbi_load(filename.c_str(), &width, &height, &bpp, STBI_rgb_alpha);
		bufflen = width * height * 4;
	}
	else {
		imageData = stbi_load(filename.c_str(), &width, &height, &bpp, STBI_rgb);
		bufflen = width * height * 3;
	}

	if (imageData == nullptr) {
		return RGBBuffer();
	}

	RGBBuffer res = RGBBuffer(width, height, useAlpha);
	res.data.clear();
	for (int i = 0; i < bufflen; i++) {
		res.data.push_back((uint8_t)imageData[i]);
	}

	stbi_image_free(imageData);

	return res;
}

bool RGBBuffer::isNull() const
{
	return ((width <= 0) || (height <= 0));
}
