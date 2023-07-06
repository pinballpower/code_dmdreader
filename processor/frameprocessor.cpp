#include "frameprocessor.hpp"

DMDFrameProcessor::DMDFrameProcessor()
{
}

DMDFrameProcessor::~DMDFrameProcessor()
{
	close();
}

DMDFrame DMDFrameProcessor::processFrame(DMDFrame& f)
{
	return f;
}

void DMDFrameProcessor::close()
{
}

bool DMDFrameProcessor::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) {
	return false;
};
