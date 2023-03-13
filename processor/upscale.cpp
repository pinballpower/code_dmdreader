#include "upscale.hpp"

//
// There are lots of pixel art upscaling algorithms available. This gives an overview:
// https://en.wikipedia.org/wiki/Pixel-art_scaling_algorithms
//

extern "C" void hqxInit(void);
extern "C" void hq2x_32(uint32_t const* src, uint32_t * dest, int width, int height);
extern "C" void hq3x_32(uint32_t const* src, uint32_t * dest, int width, int height);
extern "C" void hq4x_32(uint32_t const* src, uint32_t * dest, int width, int height);

DMDFrame Upscale::processFrame(DMDFrame& f)
{
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
			dst_width = src_width * 2; 
			dst_height = src_height * 2;   
			break;
		case Upscaler::hq3x: 
			dst_width = src_width * 3; 
			dst_height = src_height * 3;   
			break;
		case Upscaler::hq4x: 
			dst_width = src_width * 4; 
			dst_height = src_height * 4;   
			break;
	}

	// allocate destination temporary buffer
	uint32_t* dst_data = new uint32_t[dst_width * dst_height];

	switch (upscaler)
	{
		case Upscaler::hq2x:
			hq2x_32((uint32_t*)f.getPixelData().data(), dst_data, src_width, src_height);
			break;
		case Upscaler::hq3x:
			hq3x_32((uint32_t*)f.getPixelData().data(), dst_data, src_width, src_height);
			break;
			break;
		case Upscaler::hq4x:
			hq4x_32((uint32_t*)f.getPixelData().data(), dst_data, src_width, src_height);
			break;
			break;
	}

	DMDFrame result = DMDFrame(dst_width, dst_height, 32, (uint8_t*)dst_data);

	delete[] dst_data;
	return result;
}

bool Upscale::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) {
	// TODO: allow different upscalers
	upscaler = Upscaler::hq2x;

	return true;
};
