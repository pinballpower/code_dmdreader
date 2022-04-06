#pragma once

#include "dmdframe.h"

class MaskedDMDFrame : public DMDFrame {

public:

	MaskedDMDFrame();
	~MaskedDMDFrame();

	bool matches(DMDFrame &frame);

	/**
	 * Read a frame from a BMP file
	 *
	 * grayindex: offset of the color to use as the gray channel
	 * R=0, G=1, B=2
	 */
	int read_from_rgbimage(RGBBuffer* rgbdata, DMDPalette* palette, int bitperpixel = 4);

protected:

	vector <uint8_t> mask; // one byte per pixel

};
