#pragma once

#include <optional>

#include "color.hpp"

class DMDPalette {

public:
	vector<DMDColor> colors;
	int bitsperpixel;
	string name;

	DMDPalette(const DMDColor end_color = DMDColor(0xff,0,0), int bitsperpixel = 2, string name = "");
	DMDPalette(vector<uint32_t> colors, int bitsperpixel, string name = "");

	int getIndexOf(uint32_t color, bool ignore_alpha = true) const;
	int getIndexOf(uint8_t r, uint8_t g, uint8_t b) const;

	bool matchesImage(const RGBBuffer& buf) const;

	int size() const;

	static DMDPalette pd_4_ffc300();
};

const std::optional<DMDPalette> find_matching_palette(const vector<DMDPalette> palettes, const RGBBuffer buf);
vector<DMDPalette> default_palettes();
