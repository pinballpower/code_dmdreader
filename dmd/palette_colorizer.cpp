#include <boost/log/trivial.hpp>

#include "../dmd/palette.h"
#include "palette_colorizer.h"

DMDFrame color_frame(const DMDFrame frame, const DMDPalette palette)
{
	int width = frame.get_width();
	int height = frame.get_height();
	int len = width * height;

	uint8_t* colordata = new uint8_t[len*3];
	uint8_t* d = colordata;

	DMDColor c;
	DMDFrame result = DMDFrame(width, height, 24);
	for (auto px: frame.get_data()) {
		if (px > palette.size()) {
			c = DMDColor(0);
			BOOST_LOG_TRIVIAL(warning) << "[palette_colorizer] pixel value " << px << " larger than palette (" << palette.size() << ")";
		}
		else {
			c = palette.colors[px];
		}
		result.add_pixel(c.c.cols.r);
		result.add_pixel(c.c.cols.g);
		result.add_pixel(c.c.cols.b);
	}

	return result;
}
