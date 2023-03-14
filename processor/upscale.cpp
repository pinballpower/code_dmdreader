#include "upscale.hpp"

//
// There are lots of pixel art upscaling algorithms available. This gives an overview:
// https://en.wikipedia.org/wiki/Pixel-art_scaling_algorithms
//

Upscale::~Upscale() {
	delete(xbrData);
}

DMDFrame Upscale::processFrame(DMDFrame& f)
{
	xbr_params xbrParams;
	int scaleFactor = 1;

	if (f.getBitsPerPixel() != 32) {
		BOOST_LOG_TRIVIAL(info) << "[upscale] module requires at least 32bits/pixel input data, doing nothing";
		return f;
	}

	int src_width = f.getWidth();
	int src_height = f.getHeight();
	int dst_width = 0, dst_height = 0;

	switch (upscaler)
	{
		case Upscaler::hq2x: 
		case Upscaler::xbr2x:
			scaleFactor = 2;
			break;
		case Upscaler::hq3x: 
		case Upscaler::xbr3x:
			scaleFactor = 3;
			break;
		case Upscaler::hq4x: 
		case Upscaler::xbr4x:
			scaleFactor = 4;
			break;
	}

	dst_width = src_width * scaleFactor;
	dst_height = src_height * scaleFactor;


	// allocate destination temporary buffer
	uint32_t* dst_data = new uint32_t[dst_width * dst_height];


	auto src_data = f.getPixelData();
	xbrParams.data = xbrData;
	xbrParams.input = src_data.data();
	xbrParams.output = (uint8_t*)dst_data;
	xbrParams.inWidth = src_width;
	xbrParams.inHeight = src_height;
	xbrParams.inPitch = src_width * 4;
	xbrParams.outPitch = src_width * scaleFactor * 4;

	switch (upscaler)
	{
		case Upscaler::hq2x: xbr_filter_hq2x(&xbrParams); break;
		case Upscaler::hq3x: xbr_filter_hq2x(&xbrParams); break;
		case Upscaler::hq4x: xbr_filter_hq2x(&xbrParams); break;
		case Upscaler::xbr2x: xbr_filter_xbr2x(&xbrParams); break;
		case Upscaler::xbr3x: xbr_filter_xbr3x(&xbrParams); break;
		case Upscaler::xbr4x: xbr_filter_xbr4x(&xbrParams); break;
	}

	DMDFrame result = DMDFrame(dst_width, dst_height, 32, (uint8_t*)dst_data);

	delete[] dst_data;
	return result;
}

bool Upscale::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) {

	string upscalerConf = pt_source.get("upscaler", "hq2x");

	if (upscalerConf == "hq2x") {
		upscaler = Upscaler::hq2x;
	} else if (upscalerConf == "hq3x") {
		upscaler = Upscaler::hq3x;
	}
	else if (upscalerConf == "hq4x") {
		upscaler = Upscaler::hq4x;
	} 
	else if (upscalerConf == "xbr2x") {
		upscaler = Upscaler::xbr2x;
	}
	else if (upscalerConf == "xbr3x") {
		upscaler = Upscaler::xbr3x;
	}
	else if (upscalerConf == "xbr4x") {
		upscaler = Upscaler::xbr4x;
	}
	else {
		BOOST_LOG_TRIVIAL(error) << "[upscale] upscaler " << upscalerConf << " unknown, can't use upsampling";
		return false;
	}

	xbrData = new xbr_data;
	xbr_init_data(xbrData);

	return true;
};
