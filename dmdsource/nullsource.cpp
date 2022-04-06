#include "nullsource.h"

NullDMDSource::NullDMDSource()
{
}

unique_ptr<DMDFrame> NullDMDSource::next_frame(bool blocking)
{
	return unique_ptr<DMDFrame>();
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

void NullDMDSource::get_properties(SourceProperties* p)
{
	p->width = width;
	p->height = height;
	p->bitsperpixel = bitsperpixel;
}
