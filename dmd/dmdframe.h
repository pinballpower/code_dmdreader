#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "../util/crc32.h"
#include "../util/image.h"
#include "color.h"

#define  PIXVAL uint32_t  // up to 32bit/pixel

using namespace std;

class DMDFrame {

protected:

	int width;
	int height;
	int bitsperpixel;
	vector<uint8_t> data;
	/// <summary>
	/// Stores bit planes
	/// </summary>
	vector<vector <uint8_t>>planes;

public:

	DMDFrame(int width = 0, int height = 0, int bitsperpixel = 0, uint8_t* data1 = NULL);
	DMDFrame(int width, int height, int bitsperpixel, vector <uint8_t> data1);
	~DMDFrame();

	PIXVAL getPixel(int x, int y);

	bool same_size(DMDFrame &f2);
	bool equals_fast(DMDFrame &f2);

	int get_width();
	int get_height();
	int get_bitsperpixel();
	uint8_t get_pixelmask();
	uint32_t get_checksum();

	void set_size(int width, int height, int bits_per_pixel);

	const vector<uint8_t> get_data();
	const vector<uint8_t> get_plane(int bitno);

	void add_pixel(uint8_t px);

	bool is_null();
	bool is_valid();

	string str();


protected:

	void recalc_checksum();
	void init_mem(int no_of_pixels = 0);
	void copy_data(uint8_t* dat, int len);
	void calc_planes();

	int bytesperpixel();

	// cache some stuff
	int datalen;
	int rowlen;
	uint8_t pixel_mask;
	uint32_t checksum; // uses for fast equality check
};

