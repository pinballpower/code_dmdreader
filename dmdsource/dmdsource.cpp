#include "dmdsource.h"
#include "../dmd/dmdframe.h"

DMDFrame DMDSource::next_frame(bool blocking)
{
	return DMDFrame();
}

bool DMDSource::finished()
{
	return true;
}

bool DMDSource::frame_ready()
{
	return false;
}

bool DMDSource::configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) {
	return false;
}

SourceProperties DMDSource::get_properties() {
	throw std::logic_error("Function not yet implemented");	
}

int DMDSource::dropped_frames()
{
	return 0;
}

SourceProperties::SourceProperties(int width, int height, int bitsperpixel)
{
	this->width = width;
	this->height = height;
	this->bitsperpixel = bitsperpixel;
}

SourceProperties::SourceProperties(DMDFrame& f)
{
	width = f.get_width();
	height = f.get_height();
	bitsperpixel = f.get_bitsperpixel();
}
