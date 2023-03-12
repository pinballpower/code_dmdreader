#include "serumcolorizer.hpp"

#include "../external/libserum/src/serum-decode.h"


bool SerumColorizer::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	string serumfile = pt_source.get("file", "");
	int ignoreUnknownFramesTimeout = pt_source.get("ignoreUnknownFramesTimesout", 1);

	BOOST_LOG_TRIVIAL(info) << "[serumcolorizer] using " << serumfile;

	unsigned int pnocolors = 0, pntriggers = 0;
	width = height = 0;

	bool ok = Serum_LoadFile(serumfile.c_str(), &width, &height, &pnocolors, &pntriggers);
	if (ok) {
		BOOST_LOG_TRIVIAL(info) << "[serumcolorizer] loaded SERUM colorisation from " << serumfile << width << "x" << height << " " << pnocolors << " colors, " << pntriggers << " triggers";
	}
	else {
		BOOST_LOG_TRIVIAL(error) << "[serumcolorizer] couldn't load SERUM colorisation from " << serumfile;
	}

	Serum_SetIgnoreUnknownFramesTimeout(ignoreUnknownFramesTimeout);

	return ok;

}

DMDFrame SerumColorizer::processFrame(DMDFrame& f) {

	if (f.getBitsPerPixel() > 8) {
		BOOST_LOG_TRIVIAL(debug) << "[serumcolorizer] frame has already " << f.getBitsPerPixel() << " bits/pixel, not colorizing it";
		return f;
	}

	if ((f.getHeight() != height) || (f.getWidth() != width)) {
		BOOST_LOG_TRIVIAL(error) << "[serumcolorizer] dimension of colorisation and frame do not match ( " 
			<< f.getWidth() << "x" << f.getHeight() << " != " << width << "x" << height << 
			", doing nothing";
		return f;
	}

	// low level SERUM colorisation
	uint8_t* framedata = new uint8_t[width*height];
	uint8_t* dst = framedata;
	auto src = f.getPixelData();
	for (int i = 0; i < (width * height); i++) {
		*dst = src[i];
		dst++;
	}
	uint8_t palette[PALETTE_SIZE];
	uint8_t rotations[ROTATION_SIZE];
	uint32_t triggerID = 0;

	uint32_t chksum = f.getChecksum();
	bool coloredOk = Serum_Colorize(framedata, width, height, palette, rotations, &triggerID);

	if (!coloredOk)
		return f;

	// create colored data
	vector<uint8_t> colordata;
	colordata.reserve(width * height * 3);

	uint8_t* colorindex = framedata;
	uint8_t* color = NULL;
	for (int i = 0; i < width * height; i++, colorindex++) {
		color = &(palette[3 * (*colorindex)]);
		colordata.push_back(*color++); // red
		colordata.push_back(*color++); // green 
		colordata.push_back(*color++); // blue
	}

	DMDFrame res = DMDFrame(width, height, 24, colordata);

	return res;
}