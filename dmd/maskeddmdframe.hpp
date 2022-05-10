#pragma once

#include "dmdframe.hpp"
#include "palette.hpp"

class MaskedDMDFrame : public DMDFrame {

public:

	MaskedDMDFrame();
	~MaskedDMDFrame();

	bool matchesImage(const DMDFrame frame) const;

	void readFromRGBImage(const RGBBuffer rgbdata, const DMDPalette palette, int bitsperpixel);

protected:

	vector <uint8_t> mask; // one byte per pixel

};
