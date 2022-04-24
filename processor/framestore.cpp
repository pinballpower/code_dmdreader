#include "framestore.h"

FrameStore::~FrameStore()
{
}

bool FrameStore::configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	return false;
}

DMDFrame FrameStore::process_frame(DMDFrame& f)
{
	return DMDFrame();
}
