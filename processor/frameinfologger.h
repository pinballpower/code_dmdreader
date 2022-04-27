#pragma once

#include <boost/property_tree/ptree.hpp>

#include "../dmd/dmdframe.h"
#include "frameprocessor.h"

class FrameInfoLogger : public DMDFrameProcessor {

public:

	FrameInfoLogger();
	~FrameInfoLogger();
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

	virtual DMDFrame processFrame(DMDFrame &f) override;
};