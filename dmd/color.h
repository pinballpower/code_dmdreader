#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "../util/endian.h"
#include "../util/image.h"

using namespace std;

#define COLOR_VECTOR vector<DMDColor>

union colorUnion{
	uint32_t value;
	struct {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t alpha;
	} cols;
};

class DMDColor {

public:
	colorUnion c;

	DMDColor();
	DMDColor(uint32_t colors, bool revert_endian=false);
	DMDColor(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t alpha1 = 0);

	bool matches(uint8_t r1, uint8_t g1, uint8_t b1);
	bool matches(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t alpha1);
	bool matches(DMDColor c, bool ignore_alpha = true);

	uint32_t get_color_data();

};

class DMDPalette {

public:
	DMDColor* colors;
	int size;
	int bitsperpixel;
	string name;

	DMDPalette(int size, int bitsperpixel, string name="");
	DMDPalette(uint32_t* colors, int size, int bitsperpixel, string name="");
	~DMDPalette();

	int find(uint32_t color, bool ignore_alpha=true);
	int find(uint8_t r, uint8_t g, uint8_t b);

	bool matches(RGBBuffer* buf);

};

DMDPalette* find_matching_palette(vector<DMDPalette*> palettes, RGBBuffer* buf);
vector<DMDPalette*> default_palettes();
