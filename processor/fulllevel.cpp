#include <iomanip>

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "fulllevel.hpp"

int max_level_in_frame(DMDFrame f, int absoluteMaximum) {
	int maxLevel = -1;
	for (auto p : f.getPixelData()) {
		if (p == absoluteMaximum) {
			return absoluteMaximum;
		}
		if (p > maxLevel) {
			maxLevel = p;
		}
	}
	return maxLevel;
}

FullLevel::FullLevel()
{
	name = "FullLevel";
}

bool FullLevel::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	bits_per_pixel = pt_source.get("bits_per_pixel", 4);
	max_level = (1 << bits_per_pixel) - 1;

	BOOST_LOG_TRIVIAL(info) << "[fulllevel] configured for " << bits_per_pixel << "bits/pixel, levels = 0-" << max_level;

	return true;
}


DMDFrame FullLevel::processFrame(DMDFrame &f)
{
	BOOST_LOG_TRIVIAL(debug) << "[fulllevel] got frame ";

	auto frame_max = max_level_in_frame(f, max_level);
	BOOST_LOG_TRIVIAL(debug) << "[fulllevel] got frame with maximum pixel brightness " << frame_max;

	if (frame_max == 0) {
		return f;
	}
	else if (frame_max == max_level) {
		return f;
	}
	else {
		vector<uint8_t> new_pixels;

		int adjust = max_level * 256 / frame_max;
		BOOST_LOG_TRIVIAL(debug) << "[fulllevel] using multiplier " << adjust << "/256";

		for (auto px : f.getPixelData()) {
			int new_brightness = (px * adjust) / 256;
			assert(new_brightness >= 0);
			assert(new_brightness <= max_level);
			new_pixels.push_back(new_brightness);
		}
		DMDFrame res = DMDFrame(f.getWidth(), f.getHeight(), f.getBitsPerPixel(), new_pixels);
		return res;
	}
}

