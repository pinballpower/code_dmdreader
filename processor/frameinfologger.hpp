#pragma once

#include <boost/property_tree/ptree.hpp>

#include "../dmd/dmdframe.hpp"
#include "frameprocessor.hpp"

class FrameInfoLogger : public DMDFrameProcessor {

public:

	FrameInfoLogger();
	~FrameInfoLogger();
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

	virtual DMDFrame processFrame(DMDFrame &f) override;
};