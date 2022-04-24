#pragma once

#include <boost/property_tree/ptree.hpp>

#include "../dmd/dmdframe.h"
#include "frameprocessor.h"

class FrameStore : public DMDFrameProcessor {

public:

	~FrameStore();

	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);
	virtual DMDFrame process_frame(DMDFrame &f);
};