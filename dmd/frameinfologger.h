#pragma once

#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "dmdframe.h"
#include "frameprocessor.h"

class FrameInfoLogger : DMDFrameProcessor {

public:

	FrameInfoLogger();
	~FrameInfoLogger();
	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);

	virtual DMDFrame process_frame(DMDFrame &f);
};