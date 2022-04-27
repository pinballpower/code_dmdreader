#pragma once

#include "dmdframe.h"
#include "palette.h"

class MaskedDMDFrame : public DMDFrame {

public:

	MaskedDMDFrame();
	~MaskedDMDFrame();

	bool matchesImage(const DMDFrame frame) const;

	void readFromRGBImage(const RGBBuffer rgbdata, const DMDPalette palette, int bitsperpixel);

protected:

	vector <uint8_t> mask; // one byte per pixel

};
