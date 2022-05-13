#include <cstdint>
#include <iostream>
#include <vector>
#include <assert.h>

#include "color.hpp"

using namespace std;

DMDColor::DMDColor() {
	r = g = b = alpha = 0;

}

DMDColor::DMDColor(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t alpha1) {
	r = r1;
	g = g1;
	b = b1;
	alpha = alpha1;
}

DMDColor::DMDColor(uint32_t colors) {
	r = (colors >> 24) & 0xff;
	g = (colors >> 16) & 0xff;
	b = (colors >>  8) & 0xff;
	alpha = colors     & 0xff;
}

bool DMDColor::matches(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t alpha1) const {
	return ((r1 == r) && (g1 == g) && (b1 == b) && (alpha1 == alpha));
}

bool DMDColor::matches(uint8_t r1, uint8_t g1, uint8_t b1) const {
	return ((r1 == r) && (g1 == g) && (b1 == b));
}

bool DMDColor::matches(DMDColor color, bool ignore_alpha) const {
	if ((color.r == r) && (color.g == g) && (color.b == b)) {
		if (ignore_alpha) {
			return true;
		}
		else {
			return color.alpha == alpha;
		}
	}
	return false;
}

DMDColor DMDColor::fade(const DMDColor color, int fading)
{
	int r = color.r * fading / 256;
	int g = color.g * fading / 256;
	int b = color.b * fading / 256;
	return DMDColor(r,g,b);
}


