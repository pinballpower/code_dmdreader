#pragma once 

#include <cstdint>
#include <iostream>

#include "switchmode.hpp"

using namespace std;

class PaletteMapping {

public:
	static const int LENGTH = 11;
	uint32_t checksum;

	SwitchMode mode;

	uint16_t palette_index;

	/// how long is this palette active before it will be switched back to the 
	/// default palette (0: never switch back)
	uint32_t duration;

	/// byte position of the animation in FSQ/VNI file
	uint32_t offset;

	/// Is this an animation
	bool IsAnimation() const;

	PaletteMapping();
	PaletteMapping(istream& is);
};