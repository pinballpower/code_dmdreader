
#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <assert.h>

#include "../util/crc32.h"
#include "../util/bmp.h"

#include "dmdframe.h"
#include "../util/numutils.h"

DMDFrame::DMDFrame(int columns, int rows, int bitsperpixel, uint32_t* data)
{
	this->columns = columns;
	this->rows = rows;
	this->bitsperpixel = bitsperpixel;
	this->data = NULL;
	checksum = 0;
	pixel_mask = 0;

	this->init_mem(data);
}


DMDFrame::~DMDFrame() {
	if (data) {
		delete[] data;
	}
}

PIXVAL DMDFrame::getPixel(int x, int y) {
	int offset = y * rowlen + x / bitsperpixel;
	int pixoffset = 32 - (x % bitsperpixel);
	return (data[offset] >> pixoffset) & pixel_mask;
}

int DMDFrame::read_from_stream(std::ifstream& fis)
{
	if ((!fis.good()) || fis.eof()) {
		return -1;
	}

	uint8_t header[8];
	try {
		fis.read((char*)header, 8);
		rows = (header[0] << 8) + header[1];
		columns = (header[2] << 8) + header[3];
		bitsperpixel = (header[6] << 8) + header[7];
		this->init_mem(NULL);

		fis.read((char*)data, datalen*4);
		for (int i = 0; i < datalen; i++) {
			fix_uint32_endian(&data[i]);
		}
		recalc_checksum();
	}
	catch (std::ios_base::failure) {
		return -1;
	}

	return 0;
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

void DMDFrame::recalc_checksum() {
	if (data && datalen) {
		checksum = crc32buf((uint8_t*)data, datalen);
	}
	else {
		checksum = 0;
	};
}

void DMDFrame::init_mem(uint32_t* data) {
	assert((bitsperpixel <= 32) && (bitsperpixel >= 0));

	rowlen = roundup_4(columns * bitsperpixel / 8)/4;
	datalen = roundup_4(rowlen * rows);

	pixel_mask = 0xffffffff >> (32 - bitsperpixel);

	if (datalen) {

		delete[] this->data;
		this->data = new uint32_t[datalen];

		if (data) {
			memcpy_s(this->data, datalen*4, data, datalen*4);
		}
		else {
			memset(this->data, 0, datalen*4);
		}

		recalc_checksum();
	}
	else {
		this->data = NULL;
	}
}

/*
 * Internal helper funtions
 * Calculate the next pixel in a bytearray where each pixel uses bitperpixel bits
 */
uint32_t DMDFrame::get_next_pixel(uint32_t **buf, int *pixel_bit) {
	*pixel_bit -= bitsperpixel;
	if (*pixel_bit < 0) {
		*pixel_bit += 32;
		(*buf)++;
	}

	return ((**buf >> *pixel_bit) & pixel_mask);
}

void DMDFrame::calc_next_pixel(uint32_t** buf, int* pixel_bit, bool clear) {
	*pixel_bit -= bitsperpixel;
	if (*pixel_bit < 0) {
		*pixel_bit += 32;
		(*buf)++;
		if (clear) **buf = 0;
	}
}


int DMDFrame::get_width() {
	return columns;
}

int DMDFrame::get_height() {
	return rows;
}

uint32_t* DMDFrame::get_data() {
	return data;
}

uint32_t DMDFrame::get_pixelmask() {
	return pixel_mask;
}

uint32_t DMDFrame::get_checksum()
{
	return checksum;
}

int DMDFrame::get_bitsperpixel() {
	return bitsperpixel;
}

MaskedDMDFrame::MaskedDMDFrame() {
	mask = NULL;
}

MaskedDMDFrame::~MaskedDMDFrame() {
	if (mask) {
		delete[] mask;
	}
}

bool MaskedDMDFrame::matches(DMDFrame* frame) {

	if ((frame->get_bitsperpixel() != DMDFrame::bitsperpixel) ||
		(frame->get_width() != DMDFrame::columns) ||
		(frame->get_height() != DMDFrame::rows)) 
	{
		return false;
	}

	uint32_t* orig = frame->get_data();
	uint32_t* to_compare = (uint32_t*) DMDFrame::data;
	uint32_t* msk = (uint32_t*)mask;

	for (int i = 0; i < DMDFrame::datalen / 4; i++) {
		if ((*orig & *msk) != *to_compare) {
			return false;
		}
		*orig++;
		*msk++;
		*to_compare++;
	}

	return true;
}

int MaskedDMDFrame::read_from_rgbimage(RGBBuffer* rgbdata, DMDPalette* palette, int bitsperpixel) {

	assert((bitsperpixel > 0) && (bitsperpixel <= 8));

	int max_index = (1 << bitsperpixel)-1;
	uint8_t allset = (uint8_t)max_index;

	// Initialize memory
	DMDFrame::columns = rgbdata->width;
	DMDFrame::rows = rgbdata->height;
	DMDFrame::bitsperpixel = bitsperpixel;
	init_mem();
	if (mask) {
		delete[] mask;
	};
	mask = new uint32_t[this->datalen];


	// Mask calculations
	int mask_x1, mask_x2, mask_y1, mask_y2;
	mask_x1 = columns + 1;
	mask_x2 = -1;
	mask_y1 = rows + 1;
	mask_y2 = -1;

	uint8_t* rgb_src = (uint8_t*)rgbdata->data;
	uint32_t* dst = DMDFrame::data;
	int dst_bit = 32;

	bool color_not_found = false;

	for (int y = 0; y < rgbdata->height; y++) {
		for (int x = 0; x < rgbdata->width; x++, rgb_src+=3) {

			int color_index = palette->find(rgb_src[0], rgb_src[1], rgb_src[2]);

			if (color_index < 0) {
				color_not_found = true;
				color_index = 0;
			}
			else if (color_index > max_index) 
			{
				// in this case, this is a mask color
				color_index = 0;

				if (x < mask_x1) {
					mask_x1 = x;
				}
				if (x > mask_x2) {
					mask_x2 = x;
				}
				if (y < mask_y1) {
					mask_y1 = y;
				}
				if (y > mask_y2) {
					mask_y2 = y;
				}
		
			}
			else 
			{

			}

			// go to next pixel and set it to zero 
			calc_next_pixel(&dst, &dst_bit, true);

			// set the bits for this pixel
			*dst = *dst | (color_index << dst_bit);
	
		}
	}

	// Masking
	dst = mask;
	uint32_t *src = data;
	int src_bit = 32;
	dst_bit = 32;

	if ((mask_x1 <= mask_x2) && (mask_y1 <= mask_y2)) {
		// mask rectangle found
		for (int y = 0; y < rgbdata->height; y++) {
			for (int x = 0; x < rgbdata->width; x++) {
				
				// next pixel
				DMDFrame::get_next_pixel(&src, &src_bit);
				DMDFrame::calc_next_pixel(&dst, &dst_bit, true);

				if ((x <= mask_x1) || (x >= mask_x2) || (y <= mask_y1) || (y >= mask_y2)) {
					// masked
					*src &= (~(allset << src_bit)); // clear  pixels
					// mask won't be set as it is already 0
				}
				else {
					// unmasked
					*dst |= (allset << src_bit); // set mask to 1
				}
			}
		}
	}

	DMDFrame::recalc_checksum();

	return 0;
}
