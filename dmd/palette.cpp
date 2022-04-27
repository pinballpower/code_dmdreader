#include <cassert>

#include "palette.h"


DMDPalette::DMDPalette(const DMDColor end_color, int bitsperpixel, string name)
{
	this->bitsperpixel = bitsperpixel;
	this->name = name;
	colors.clear();

	int num_colors = 1 << bitsperpixel;

	for (int i = 0; i < num_colors; i++) {
		int fading = 256 * i / num_colors;
		colors.push_back(DMDColor::fade(end_color, fading));
	}

}

DMDPalette::DMDPalette(vector<uint32_t> colors, int bitsperpixel, string name1)
{
	assert(colors.size() >= (int(1) << bitsperpixel));

	this->colors.clear();
	for (const auto c : colors) {
		DMDColor dmdcolor = DMDColor(c);
		this->colors.push_back(c);
	}

	this->name = name;
}

int DMDPalette::index_of(uint32_t color, bool ignore_alpha) const {
	for (int i = 0; i < colors.size(); i++) {
		if (colors[i].matches(color, ignore_alpha)) {
			return i;
		}
	}
	return -1;
}

int DMDPalette::index_of(uint8_t r, uint8_t g, uint8_t b) const {
	for (int i = 0; i < colors.size(); i++) {
		if (colors[i].matches(r, g, b)) {
			return i;
		}
	}
	return -1;
}

bool DMDPalette::matches(const RGBBuffer& buf) const
{
	const vector <uint8_t> data = buf.getData();
	for (int i = 0; i < data.size(); i += 3) {
		uint8_t r = data[i];
		uint8_t g = data[i + 1];
		uint8_t b = data[i + 2];
		bool color_found = false;

		for (const auto c : colors) {
			if (c.matches(r, g, b)) {
				color_found = true;
				break;
			}
		}

		if (!(color_found)) {
			return false;
		}
	}
	return true;
}

int DMDPalette::size() const
{
	return colors.size();
}



const std::optional<DMDPalette> find_matching_palette(const vector<DMDPalette> palettes, const RGBBuffer buf)
{
	for (const auto palette : palettes) {
		if (palette.matches(buf)) {
			return palette;
		}
	}

	return std::nullopt;
}


vector<DMDPalette> default_palettes() {
	vector<DMDPalette> res = vector<DMDPalette>();

	vector<uint32_t> pd_4_orange_masked_data{
		0x00000000, 0x11050000, 0x22090000, 0x330e0000,
		0x44120000, 0x55170000, 0x661C0000, 0x77200000,
		0x88000000, 0x99000000, 0xaa000000, 0xbb000000,
		0xcc370000, 0xdd3c0000, 0xee400000, 0xff450000,
		0xfd00fd00 };
	DMDPalette pd_4_orange_mask = DMDPalette(pd_4_orange_masked_data, 4, "pd_4_orange_mask");

	res.push_back(pd_4_orange_mask);

	return res;
}