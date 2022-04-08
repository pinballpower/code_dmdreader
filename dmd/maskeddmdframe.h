#pragma once

#include "dmdframe.h"
#include "palette.h"

class MaskedDMDFrame : public DMDFrame {

public:

	MaskedDMDFrame();
	~MaskedDMDFrame();

	bool matches(const DMDFrame frame) const;

	void read_from_rgbimage(const RGBBuffer rgbdata, const DMDPalette palette, int bitsperpixel);

protected:

	vector <uint8_t> mask; // one byte per pixel

};
