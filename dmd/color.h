#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "../util/endian.h"
#include "../util/image.h"

using namespace std;

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

	bool matchesImage(uint8_t r1, uint8_t g1, uint8_t b1) const;
	bool matchesImage(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t alpha1) const;
	bool matchesImage(DMDColor c, bool ignore_alpha = true) const;

	uint32_t getColorData() const;

	static DMDColor fade(const DMDColor color, int fading);

};
