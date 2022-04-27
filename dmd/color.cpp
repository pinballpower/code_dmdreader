#include <cstdint>
#include <iostream>
#include <vector>
#include <assert.h>

#include "color.h"

using namespace std;

DMDColor::DMDColor() {
	c.value = 0;

}

DMDColor::DMDColor(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t alpha1) {
	c.cols.r = r1;
	c.cols.g = g1;
	c.cols.b = b1;
	c.cols.alpha = alpha1;
}

DMDColor::DMDColor(uint32_t colors, bool revert_endian) {
	if (revert_endian) {
		c.cols.r = colors && 0xff;
		c.cols.g = (colors >> 8) && 0xff;
		c.cols.g = (colors >> 16) && 0xff;
		c.cols.alpha = (colors >> 24) && 0xff;
	}
	else {
		c.value = colors;
	}
}

bool DMDColor::matchesImage(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t alpha1) const {
	return ((r1 == c.cols.r) && (g1 == c.cols.g) && (b1 == c.cols.b) && (alpha1 == c.cols.alpha));
}

bool DMDColor::matchesImage(uint8_t r1, uint8_t g1, uint8_t b1) const {
	return ((r1 == c.cols.r) && (g1 == c.cols.g) && (b1 == c.cols.b));
}

bool DMDColor::matchesImage(DMDColor color, bool ignore_alpha) const {
	if ((color.c.cols.r == c.cols.r) && (color.c.cols.g == c.cols.g) && (color.c.cols.b == c.cols.b)) {
		if (ignore_alpha) {
			return true;
		}
		else {
			return color.c.cols.alpha == c.cols.alpha;
		}
	}
	return false;
}

uint32_t DMDColor::getColorData() const
{
	return c.value;
}

DMDColor DMDColor::fade(const DMDColor color, int fading)
{
	int r = color.c.cols.r * fading / 256;
	int g = color.c.cols.g * fading / 256;
	int b = color.c.cols.b * fading / 256;
	return DMDColor(r,g,b);
}


