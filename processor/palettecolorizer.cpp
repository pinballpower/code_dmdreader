#include "palettecolorizer.h"

DMDFrame PaletteColorizer::processFrame(DMDFrame& f)
{
	if (f.getBitsPerPixel() > 8) {
		BOOST_LOG_TRIVIAL(debug) << "[palettecolorizer] frame is already colored, doing nothing";

		return std::move(f);
	}

	int width = f.getWidth();
	int height = f.getHeight();
	int len = width * height;

	uint8_t* colordata = new uint8_t[len * 3];
	uint8_t* d = colordata;

	DMDColor c;
	DMDFrame result = DMDFrame(width, height, 24);
	for (auto px : f.getPixelData()) {
		if (px > palette.size()) {
			c = DMDColor(0);
			BOOST_LOG_TRIVIAL(warning) << "[palettecolorizer] pixel value " << px << " larger than palette (" << palette.size() << ")";
		}
		else {
			c = palette[px];
		}
		result.appendPixel(c.c.cols.r);
		result.appendPixel(c.c.cols.g);
		result.appendPixel(c.c.cols.b);
	}

	return result;
}

bool PaletteColorizer::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
    int numcolors = pt_source.get("colors", 16);

	BOOST_LOG_TRIVIAL(info) << "[palettecolorizer] using " << numcolors << "colors";

	palette.clear();
	palette.reserve(numcolors);

	uint8_t red = pt_source.get("red", 0xff);
	uint8_t green = pt_source.get("green", 0xc3);
	uint8_t blue = pt_source.get("blue", 0x00);

	DMDColor mycolor = DMDColor(red, green, blue, 0);
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
