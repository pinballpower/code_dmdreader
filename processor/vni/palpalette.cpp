#include <iostream>
#include <fstream>

#include <boost/log/trivial.hpp>

#include "palpalette.hpp"
#include "streamhelper.hpp"
#include "../../dmd/color.hpp"

using namespace std;

PalPalette::PalPalette() {
	index=0;
	type = 0;
}

PalPalette::~PalPalette()
{
	colors.clear();
}

PalPalette::PalPalette (istream& is) {
	index = read_u16_be(is);
	BOOST_LOG_TRIVIAL(trace) << "[palette] offset " << is.tellg() << " read index as " << index;
	int num_colors = read_u16_be(is);
	BOOST_LOG_TRIVIAL(trace) << "[palette] offset " << is.tellg() << " read numcolors as " << num_colors;
	type = read_u8(is);
	BOOST_LOG_TRIVIAL(trace) << "[palette] offset " << is.tellg() << " read type as " << ++type;
	colors.reserve(num_colors);
	for (int i = 0, j=0; i < num_colors; i++, j+=3) {
		uint8_t r, g, b;
		r = read_u8(is);
		g = read_u8(is);
		b = read_u8(is);
		colors.push_back(DMDColor(r,g,b));
	}
	BOOST_LOG_TRIVIAL(trace) << "[palette] offset " << is.tellg() << " read " << num_colors*3 << " bytes of color data ((" << num_colors << " colors)";
}



bool PalPalette::isDefault() const
{
	return (type == 1 || type == 2);
}

bool PalPalette::isPersistent() const
{
	return (type == 1);
}

const vector<DMDColor> Palette::get_colors() const
{
	return colors;
}
