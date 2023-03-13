#include "upscalex2.hpp"

//
// There are lots of pixel art upscaling algorithms available. This gives an overview:
// https://en.wikipedia.org/wiki/Pixel-art_scaling_algorithms
//

DMDFrame UpscaleX2::processFrame(DMDFrame& f)
{
	return f;
}

bool UpscaleX2::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) {
	return true;
};
