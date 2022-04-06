#include "dmdsource.h"
#include "../dmd/dmdframe.h"

unique_ptr<DMDFrame> DMDSource::next_frame(bool blocking)
{
	return std::unique_ptr<DMDFrame>(nullptr);
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

void DMDSource::get_properties(SourceProperties* p) {
	p->width = p->height = p->bitsperpixel = 0;
}
