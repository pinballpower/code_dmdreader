#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include "../util/crc32.h"
#include "../util/image.h"
#include "color.h"

#define  PIXVAL uint32_t  // up to 32bit/pixel

using namespace std;

class DMDFrame {

protected:

	int columns;
	int rows;
	int bitsperpixel;
	uint32_t* data;

public:

	DMDFrame(int columns = 0, int rows = 0, int bitsperpixel = 0, uint32_t* data1 = NULL);
	~DMDFrame();

	PIXVAL getPixel(int x, int y);

	int read_from_stream(std::ifstream& fis);

	bool same_size(DMDFrame* f2);
	bool equals_fast(DMDFrame* f2);

	int get_width();
	int get_height();
	int get_bitsperpixel();
	uint32_t get_pixelmask();
	uint32_t get_checksum();

	uint32_t* get_data();
	string str();

protected:

	void recalc_checksum();

	void init_mem(uint32_t* data1 = NULL);

	// cache some stuff
	int datalen;
	int rowlen;
	uint32_t pixel_mask;
	uint32_t checksum; // uses for fast equality check

	uint32_t get_next_pixel(uint32_t** buf, int* pixel_bit);
	void calc_next_pixel(uint32_t** buf, int* pixel_bit, bool clear = false);
};

class MaskedDMDFrame : DMDFrame {

public:

	MaskedDMDFrame();
	~MaskedDMDFrame();

	bool matches(DMDFrame* f);

	/**
	 * Read a frame from a BMP file
	 *
	 * grayindex: offset of the color to use as the gray channel
	 * R=0, G=1, B=2
	 */
	int read_from_rgbimage(RGBBuffer* rgbdata, DMDPalette* palette, int bitperpixel = 4);

private:

	uint32_t* mask;

};

