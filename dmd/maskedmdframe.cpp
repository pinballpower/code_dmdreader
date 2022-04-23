#include <cassert>

#include "maskeddmdframe.h"


MaskedDMDFrame::MaskedDMDFrame() {
}

MaskedDMDFrame::~MaskedDMDFrame() {
}

bool MaskedDMDFrame::matches(const DMDFrame frame) const {

	if (frame.get_data().size() != data.size()) {
		return false;
	}

	auto myData = data.cbegin();
	auto maskData = mask.cbegin();
	const auto frameData = frame.get_data();
	auto otherData = frameData.cbegin();

	for (; myData != data.cend(); myData++, otherData++, maskData++) 
	{
		// if the pixel is masked, don't compare
		if (*maskData == 0) {
			continue;
		}

		if (*myData != *otherData) {
			return false;
		}
	}

	return true;
}

void MaskedDMDFrame::read_from_rgbimage(const RGBBuffer rgbdata, const DMDPalette palette, int bitsperpixel) {

	assert((bitsperpixel > 0) && (bitsperpixel <= 8));

	int max_index = (1 << bitsperpixel) - 1;
	uint8_t allset = (uint8_t)max_index;

	// Initialize memory
	DMDFrame::width = rgbdata.width;
	DMDFrame::height = rgbdata.height;
	DMDFrame::bitsperpixel = bitsperpixel;
	init_mem();

	// Mask calculations
	int mask_x1, mask_x2, mask_y1, mask_y2;
	mask_x1 = width + 1;
	mask_x2 = -1;
	mask_y1 = height + 1;
	mask_y2 = -1;
	mask.clear();

	vector<uint8_t> rgb_src = rgbdata.get_data();
	int dst_bit = 32;

	bool color_not_found = false;

	int i = 0;
	for (int y = 0; y < rgbdata.height; y++) {
		for (int x = 0; x < rgbdata.width; x++, i+=3) {

			int color_index = palette.index_of(rgb_src[i], rgb_src[i+1], rgb_src[i+2]);

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

			data.push_back(color_index);

		}
	}

	// Masking
	mask.clear();
	mask.reserve(width*height);
	if ((mask_x1 <= mask_x2) && (mask_y1 <= mask_y2)) {
		// mask rectangle found
		for (int y = 0; y < rgbdata.height; y++) {
			for (int x = 0; x < rgbdata.width; x++) {
				if ((x <= mask_x1) || (x >= mask_x2) || (y <= mask_y1) || (y >= mask_y2)) {
					// masked (do not use for comparisson)
					mask.push_back(0);
				}
				else {
					// unmasked
					mask.push_back(0xff);
				}
			}
		}
	}

	DMDFrame::get_checksum(true); // recalculate checksum
}
