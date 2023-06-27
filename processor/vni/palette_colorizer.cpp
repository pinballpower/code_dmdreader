#include <boost/log/trivial.hpp>

#include "../dmd/palette.hpp"
#include "palette_colorizer.hpp"

DMDFrame color_frame(const DMDFrame frame, const DMDPalette palette)
{
	int width = frame.getWidth();
	int height = frame.getHeight();
	int len = width * height;

	uint8_t* colordata = new uint8_t[len*3];
	uint8_t* d = colordata;

	DMDColor c;
	DMDFrame result = DMDFrame(width, height, 24);
	for (auto px: frame.getPixelData()) {
		if (px > palette.size()) {
			c = DMDColor(0);
			BOOST_LOG_TRIVIAL(warning) << "[palette_colorizer] pixel value " << px << " larger than palette (" << palette.size() << ")";
		}
		else {
			c = palette.colors[px];
		}
		result.appendPixel(c.c.cols.r);
		result.appendPixel(c.c.cols.g);
		result.appendPixel(c.c.cols.b);
	}

	return result;
}
