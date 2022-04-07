#include "nullsource.h"

NullDMDSource::NullDMDSource()
{
}

DMDFrame NullDMDSource::next_frame(bool blocking)
{
	return DMDFrame(width, height, bitsperpixel);
}

bool NullDMDSource::finished()
{
	return false;
}

bool NullDMDSource::frame_ready()
{
	return true;
}

bool NullDMDSource::configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	bitsperpixel = pt_source.get("bitsperpixel", 2);
	width = pt_source.get("width", 128);
	height = pt_source.get("width", 128);
	return true;
}

SourceProperties NullDMDSource::get_properties()
{
	return SourceProperties(width, height, bitsperpixel);
}
