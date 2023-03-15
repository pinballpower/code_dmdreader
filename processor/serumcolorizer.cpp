#include "serumcolorizer.hpp"

bool SerumColorizer::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	string serumfile = pt_source.get("file", "");
	int ignoreUnknownFramesTimeout = pt_source.get("ignoreUnknownFramesTimesout", 30);

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

	if ((width > SERUM_MAXWIDTH) || (height > SERUM_MAXHEIGHT)) {
		BOOST_LOG_TRIVIAL(info) << "[serumcolorizer] dimensions too big, not using this colorisation";
		width = 0;
		height = 0;
		ok = false;
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
	uint32_t checksum = f.getChecksum();
	bool coloredOk = false;
	if (checksum != checksumLastFrame) {
		uint8_t* dst = srcbuffer;
		auto src = f.getPixelData();
		for (int i = 0; i < (width * height); i++) {
			*dst = src[i];
			dst++;
		}
		
		uint32_t triggerID = 0;

		checksumLastFrame = checksum;
		coloredOk = Serum_Colorize(srcbuffer, width, height, palette, rotations, &triggerID);
	}
	else {
		// if the source frame hasn't changed, just re-use the previous coloriation data, but apply color rotations
		coloredOk = true;
		Serum_ApplyRotations(palette, rotations);
	}

	if (!coloredOk)
		return f;

	// create colored data
	vector<uint8_t> colordata;
	colordata.reserve(width * height * 4);

	uint8_t* colorindex = srcbuffer;
	uint8_t* color = NULL;
	for (int i = 0; i < width * height; i++, colorindex++) {
		color = &(palette[3 * (*colorindex)]);
		colordata.push_back(*color++); // red
		colordata.push_back(*color++); // green 
		colordata.push_back(*color++); // blue
		colordata.push_back(0);        // alpha
	}

	DMDFrame res = DMDFrame(width, height, 32, colordata);

	return res;
}