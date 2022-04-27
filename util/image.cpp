#include <cassert>

#include "image.h"

RGBBuffer::RGBBuffer()
{
	width = height = 0;
}

RGBBuffer::RGBBuffer(int width, int height)
{
	assert((width >= 0) && (height >= 0));

	this->width = width;
	this->height = height;

	// initialize an empty picture
	data = vector<uint8_t>(width * height * 3);
}

RGBBuffer::~RGBBuffer()
{
}

const vector<uint8_t> RGBBuffer::getData() const
{
	return data;
}

void RGBBuffer::setPixel(int x, int y, rgb rgbdata)
{
	int offset = (x + y * width) * 3;
	data[offset] = rgbdata.r;
	data[offset+1] = rgbdata.g;
	data[offset+2] = rgbdata.b;
}

void RGBBuffer::setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	int offset = (x + y * width) * 3;
	data[offset] = r;
	data[offset+1] = g;
	data[offset+2] = b;
}
