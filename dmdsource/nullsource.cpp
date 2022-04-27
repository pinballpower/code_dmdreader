#include "nullsource.h"

NullDMDSource::NullDMDSource()
{
}

DMDFrame NullDMDSource::getNextFrame()
{
	return DMDFrame(width, height, bitsperpixel);
}

bool NullDMDSource::isFinished()
{
	return false;
}

bool NullDMDSource::isFrameReady()
{
	return true;
}

bool NullDMDSource::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	bitsperpixel = pt_source.get("bitsperpixel", 2);
	width = pt_source.get("width", 128);
	height = pt_source.get("width", 128);
	return true;
}

SourceProperties NullDMDSource::getProperties()
{
	return SourceProperties(width, height, bitsperpixel);
}
