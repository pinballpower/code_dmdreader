#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "../util/image.hpp"

using namespace std;

class DMDColor {

public:
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t alpha;

	DMDColor();
	DMDColor(uint32_t colors);
	DMDColor(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t alpha1 = 0);

	bool matchesImage(uint8_t r1, uint8_t g1, uint8_t b1) const;
	bool matchesImage(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t alpha1) const;
	bool matchesImage(DMDColor c, bool ignore_alpha = true) const;

	static DMDColor fade(const DMDColor color, int fading);

};
