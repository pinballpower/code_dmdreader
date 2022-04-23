#include "palettecolorizer.h"

DMDFrame PaletteColorizer::process_frame(DMDFrame& f)
{
	int width = f.get_width();
	int height = f.get_height();
	int len = width * height;

	uint8_t* colordata = new uint8_t[len * 3];
	uint8_t* d = colordata;

	DMDColor c;
	DMDFrame result = DMDFrame(width, height, 24);
	for (auto px : f.get_data()) {
		if (px > palette.size()) {
			c = DMDColor(0);
			BOOST_LOG_TRIVIAL(warning) << "[palettecolorizer] pixel value " << px << " larger than palette (" << palette.size() << ")";
		}
		else {
			c = palette[px];
		}
		result.add_pixel(c.c.cols.r);
		result.add_pixel(c.c.cols.g);
		result.add_pixel(c.c.cols.b);
	}

	return result;
}

bool PaletteColorizer::configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
    int numcolors = pt_source.get("colors", 16);

	BOOST_LOG_TRIVIAL(info) << "[simplecolorizer] using " << numcolors << "colors";

	palette.clear();
	palette.reserve(numcolors);

	DMDColor mycolor = DMDColor(0xff, 0xc3, 0x00, 0x00);
	int step = 256 / (numcolors-1);
	int fade = 0;

	for (int i = 0; i < numcolors; i++) {
		if (i == numcolors - 1) {
			fade = 256;
		}
		DMDColor c = DMDColor::fade(mycolor, fade);
		palette.push_back(c);
		fade += step;
	}

	return true;

}
