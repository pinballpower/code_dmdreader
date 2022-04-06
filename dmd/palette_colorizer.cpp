#include <boost/log/trivial.hpp>

#include "palette_colorizer.h"

DMDFrame* color_frame(DMDFrame* frame, COLOR_VECTOR& colors)
{
	int width = frame->get_width();
	int height = frame->get_height();
	int len = width * height;

	uint8_t* colordata = new uint8_t[len*3];
	uint8_t* d = colordata;

	DMDColor c;
	DMDFrame* result = new DMDFrame(width, height, 24);
	for (auto px: frame->get_data()) {
		if (px > colors.size()) {
			c = DMDColor(0);
			BOOST_LOG_TRIVIAL(warning) << "[palette_colorizer] pixel value " << px << " larger than palette (" << colors.size() << ")";
		}
		else {
			c = colors[px];
		}
		result->add_pixel(c.c.cols.r);
		result->add_pixel(c.c.cols.g);
		result->add_pixel(c.c.cols.b);
	}

	return result;
}
