#pragma once

#include <map>

#include <boost/property_tree/ptree.hpp>

#include "../dmd/dmdframe.h"
#include "frameprocessor.h"

class FrameInfoLogger : public DMDFrameProcessor {

public:

	FrameInfoLogger();
	~FrameInfoLogger();
	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);

	virtual DMDFrame process_frame(DMDFrame &f);
};