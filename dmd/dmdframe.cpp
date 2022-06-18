
#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <cassert>

#include <boost/log/trivial.hpp>

#include "../util/crc32.hpp"

#include "dmdframe.hpp"

DMDFrame::DMDFrame(int width, int height, int bitsperpixel, uint8_t* data, bool packed)
{
	this->width = width;
	this->height = height;
	this->bitsperpixel = bitsperpixel;
	checksum = 0;
	pixel_mask = 0;
	initMemory();
	if (data != nullptr) {
		if (packed) {
			this->copyPackedPixelData(data, width * height * getBytesPerPixel(),bitsperpixel);
		}
		else {
			this->copyPixelData(data, width * height * getBytesPerPixel());
		}
	}
}

DMDFrame::DMDFrame(int columns, int rows, int bitsperpixel, vector<uint8_t> data)
{
	this->width = columns;
	this->height = rows;
	this->bitsperpixel = bitsperpixel;
	checksum = 0;
	pixel_mask = 0;
	initMemory();
	this->data = std::move(data);
}

DMDFrame::DMDFrame(const RGBBuffer rgbBuffer)
{
	this->width = rgbBuffer.width;
	this->height = rgbBuffer.height;
	this->bitsperpixel = 24;
	checksum = 0;
	pixel_mask = 0;
	initMemory();
	this->data = rgbBuffer.getData();
}


DMDFrame::~DMDFrame() {
}

PIXVAL DMDFrame::getPixel(int x, int y) {
	int offset = y * rowlen + x / bitsperpixel;
	return (data[offset]);
}

bool DMDFrame::hasSameSize(const DMDFrame& f2) const{
	return ((width == f2.width) && (height == f2.height) && (bitsperpixel == f2.bitsperpixel));
}

bool DMDFrame::hasSameSizeAndChecksum(const DMDFrame& f2) const {
	if (this->hasSameSize(f2)) {
		return checksum == f2.checksum;
	}
	else {
		return false;
	}
}

std::string DMDFrame::asString() {
	char cs[8];
	snprintf(cs, sizeof(cs), "%08x", checksum);
	return "DMDFrame(" + std::to_string(width) + "x" + std::to_string(height) + "," + std::to_string(bitsperpixel) + "bpp, checksum=" + cs + ")";
}

/// <summary>
/// Return 1 one-bit plane 
/// </summary>
/// <param name="bitno">The plane number 0: LSB</param>
/// <returns></returns>
const vector<uint8_t> DMDFrame::getPlaneData(int bitno)
{
	if (planes.size() < bitsperpixel) {
		calculatePlanes();
	}
	return planes[bitno];
}

RGBBuffer DMDFrame::createRGBBufferFromFrame() const
{
	bool alpha = false;
	if (bitsperpixel == 32) {
		alpha = true;
	}
	RGBBuffer result = RGBBuffer(width, height, alpha);

	auto data = this->getPixelData().begin();
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (bitsperpixel <= 8) {
				result.setPixel(x, y, *data, *data, *data);
				data++;
			}
			else if (bitsperpixel == 24) {
				uint8_t r = *data;
				data++;
				uint8_t g = *data;
				data++;
				uint8_t b = *data;
				data++;
				result.setPixel(x, y,r,g,b);
			}
			else if (bitsperpixel == 32) {
				uint8_t r = *data;
				data++;
				uint8_t g = *data;
				data++;
				uint8_t b = *data;
				data++;
				uint8_t alpha = *data;
				data++;
				result.setPixel(x, y, r, g, b, alpha);
			}
			else {
				BOOST_LOG_TRIVIAL(info) << "[DMDFrame] " << bitsperpixel << " unsupported, can't create RGBBuffer";
				return result;
			}
		}
	}
	return result;
}

void DMDFrame::appendPixel(uint8_t px)
{
	data.push_back(px);
	// invalidate checksum, but don't recalculate now, there might be more pixels coming
	checksum = 0;

}

int DMDFrame::getBytesPerPixel() const {
	return  (bitsperpixel + 7) / 8;
}

bool DMDFrame::isNull() const
{
	return ((width == 0) && (height == 0));
}

bool DMDFrame::isValid() const
{
	return data.size() == (width * height * getBytesPerPixel());

}

void DMDFrame::initMemory(int no_of_pixels) {
	assert(((bitsperpixel <= 8) && (bitsperpixel >= 0)) || (bitsperpixel == 24) || (bitsperpixel == 32));

	if (bitsperpixel <= 8) {
		rowlen = width;
	}
	else if (bitsperpixel == 24) {
		rowlen = width * 3;
	}
	else if (bitsperpixel == 32) {
		rowlen = width * 4;
	}
	datalen = rowlen * width;

	pixel_mask = 0xff >> (8 - bitsperpixel);

	// clear old data
	data.clear();
	data.reserve(no_of_pixels);

	// clear planes
	planes.clear();
	planes.reserve(bitsperpixel);

	checksum_valid = false;
}

void DMDFrame::copyPixelData(uint8_t* dat, int len) {
	for (int i = 0; i < len; i++, dat++) {
		data.push_back(*dat);
	}
}

/// <summary>
/// Initialize a DMDFrame from packed data. 
/// </summary>
/// <param name="packedData">A pointer to an array of bytes. Length should be numPixels / 8 * bitsPerPixel</param>
/// <param name="numPixels">The number of pixels</param>
/// <param name="bitsperpixel"></param>
void DMDFrame::copyPackedPixelData(uint8_t* packedData, int numPixels, int bitsperpixel)
{
	assert((bitsperpixel >= 1) && (bitsperpixel <= 8));

	uint8_t bitmask = (1 << bitsperpixel) - 1;

	int currentBit = 8;
	for (int i = 0; i < numPixels; i++) {
		uint8_t px;
		currentBit -= bitsperpixel;

		if (currentBit < 0) {
			currentBit += 8;
			packedData++;
		}

		px = (*packedData >> currentBit)& bitmask;

		data.push_back(px);
	}
}

void DMDFrame::calculatePlanes()
{
	if (planes.size() > 0) {
		planes.clear();
	}
	for (int i = 0; i < bitsperpixel; i++) {
		vector<uint8_t> plane = vector<uint8_t>();
		plane.reserve(width * height / 8);
		planes.push_back(plane);
	}

	uint8_t bit = 8;
	for (auto p : data) {
		// every 8 pixels, add a byte to the planes
		if (bit >= 8) {
			bit = 0;
			for (int i = 0; i < bitsperpixel; i++) {
				planes[i].push_back(0);
			}
		}
		for (uint8_t i = 0, mask = 1; i < bitsperpixel; i++, mask << 1) {
			if ((p & mask) != 0) {
				planes[i].back() = (planes[i].back() << 1) | 1;
			}
			else {
				planes[i].back() = (planes[i].back() << 1);
			}
		}
		bit++;
	}
}

int DMDFrame::getWidth() const {
	return width;
}

int DMDFrame::getHeight() const {
	return height;
}

const vector<uint8_t> DMDFrame::getPixelData() const {
	return data;
}

uint8_t DMDFrame::getPixelMask() const {
	return pixel_mask;
}

uint32_t DMDFrame::getChecksum(bool recalc) const
{
	// In case of heavy concurrency, it might be better to use a mutex instead of an atomic checksum variable. 
	// However, in this application no concurrent access to the checksum variable is expected. Therefore, atomic 
	// is used as it is more lightweight
	if (!(checksum_valid) || recalc) {
		checksum = crc32vect(data);
		checksum_valid = true;
	}		
	return checksum;

}

/// <summary>
/// Reset DMDFrame to the given size. If the size is different, the data will be removed
/// </summary>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="bits_per_pixel"></param>
void DMDFrame::setSize(int width, int height, int bits_per_pixel)
{
	if ((this->width == width) && (this->height == height) && (this->bitsperpixel == bits_per_pixel)) {
		return;
	}

	this->width = width;
	this->height = height;
	this->bitsperpixel = bits_per_pixel;
	initMemory();
}

int DMDFrame::getBitsPerPixel() const {
	return bitsperpixel;
}
