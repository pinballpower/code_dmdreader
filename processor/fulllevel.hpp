#pragma once

#include <boost/property_tree/ptree.hpp>

#include "../dmd/dmdframe.hpp"
#include "frameprocessor.hpp"

class FullLevel : public DMDFrameProcessor {

public:

	FullLevel();

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;
	virtual DMDFrame processFrame(DMDFrame &f) override;

private:
	int bits_per_pixel = 4;
	int max_level = 15;
};