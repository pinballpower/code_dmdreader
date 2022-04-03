#include <boost/log/trivial.hpp>

#include "palette_colorizer.h"

DMDFrame* color_frame(DMDFrame* frame, COLOR_VECTOR& colors)
{
	int width = frame->get_width();
	int height = frame->get_height();
	int len = width * height;

	uint32_t* colordata = new uint32_t[len];
	uint32_t* d = colordata;

	frame->start_pixel_loop();
	DMDColor c;
	for (int i = 0; i < len; i++) {
		uint32_t pxval = frame->get_next_pixel();
		if (pxval > colors.size()) {
			c=DMDColor(0);
			BOOST_LOG_TRIVIAL(warning) << "[palette_colorizer] pixel value " << pxval << " larger than palette (" << colors.size() << ")";
		}		
		else {
			c = colors[pxval];
		}
		*d = c.get_color_data();
		d++;
	}

	DMDFrame* result = new DMDFrame(width, height, 32, colordata, false);

	return result;
}
