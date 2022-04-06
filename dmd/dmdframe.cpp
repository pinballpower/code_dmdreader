
#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <cassert>

#include "../util/crc32.h"
#include "../util/bmp.h"

#include "dmdframe.h"
#include "../util/numutils.h"

DMDFrame::DMDFrame(int columns, int rows, int bitsperpixel, uint8_t* data)
{
	this->columns = columns;
	this->rows = rows;
	this->bitsperpixel = bitsperpixel;
	checksum = 0;
	pixel_mask = 0;
	init_mem();
	if (data != nullptr) {
		this->copy_data(data, rows * columns);
	}
}

DMDFrame::DMDFrame(int columns, int rows, int bitsperpixel, vector<uint8_t> data)
{
	this->columns = columns;
	this->rows = rows;
	this->bitsperpixel = bitsperpixel;
	checksum = 0;
	pixel_mask = 0;
	init_mem();
}


DMDFrame::~DMDFrame() {
}

PIXVAL DMDFrame::getPixel(int x, int y) {
	int offset = y * rowlen + x / bitsperpixel;
	return (data[offset]);
}

bool DMDFrame::same_size(DMDFrame* f2) {
	return ((columns = f2->columns) && (rows = f2->rows) && (bitsperpixel = f2->bitsperpixel));
}

bool DMDFrame::equals_fast(DMDFrame* f2) {
	if (this->same_size(f2)) {
		return checksum == f2->checksum;
	}
	else {
		return false;
	}
}

std::string DMDFrame::str() {
	char cs[8];
	snprintf(cs, sizeof(cs), "%08x", checksum);
	return "DMDFrame(" + std::to_string(columns) + "x" + std::to_string(rows) + "," + std::to_string(bitsperpixel) + "bpp, checksum=" + cs + ")";
}

/// <summary>
/// Return 1 one-bit plane 
/// </summary>
/// <param name="bitno">The plane number 0: LSB</param>
/// <returns></returns>
const vector<uint8_t> DMDFrame::get_plane(int bitno)
{
	if (planes.size() < bitsperpixel) {
		calc_planes();
	}
	return planes[bitno];
}

void DMDFrame::add_pixel(uint8_t px)
{
	data.push_back(px);
	// invalidate checksum, but don't recalculate now, there might be more pixels coming
	checksum = 0;

}

void DMDFrame::recalc_checksum() {
	checksum = crc32vect(data);
}

void DMDFrame::init_mem(int no_of_pixels) {
	assert(((bitsperpixel <= 8) && (bitsperpixel >= 0)) || (bitsperpixel==24) || (bitsperpixel==32));

	if (bitsperpixel <= 8) {
		rowlen = columns;
	} else if (bitsperpixel == 24) {
		rowlen = columns * 3; 
	}
	else if (bitsperpixel == 32) {
		rowlen = columns * 4;
	}
	datalen = rowlen * rows;

	pixel_mask = 0xff >> (8 - bitsperpixel);

	// clear old data
	data.clear();
	data.reserve(no_of_pixels);

	// clear planes
	planes.clear();
	planes.reserve(bitsperpixel);

	checksum = 0;
}

void DMDFrame::copy_data(uint8_t* dat, int len) {
	for (int i = 0; i < len; i++, dat++) {
		data.push_back(*dat);
	}
}

void DMDFrame::calc_planes()
{
	if (planes.size() > 0) {
		planes.clear();
	}
	for (int i = 0; i < bitsperpixel; i++) {
		vector<uint8_t> plane = vector<uint8_t>();
		plane.reserve(rows * columns / 8);
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
		for (uint8_t i = 0, mask=1 ; i < bitsperpixel; i++, mask << 1) {
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

int DMDFrame::get_width() {
	return columns;
}

int DMDFrame::get_height() {
	return rows;
}

const vector<uint8_t> DMDFrame::get_data() {
	return data;
}

uint8_t DMDFrame::get_pixelmask() {
	return pixel_mask;
}

uint32_t DMDFrame::get_checksum()
{
	return checksum;
}

int DMDFrame::get_bitsperpixel() {
	return bitsperpixel;
}
