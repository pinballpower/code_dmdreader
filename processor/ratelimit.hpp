#pragma once

#include <boost/property_tree/ptree.hpp>

#include "../dmd/dmdframe.hpp"
#include "frameprocessor.hpp"

class RateLimit : public DMDFrameProcessor {

public:
	RateLimit();

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;
	virtual DMDFrame processFrame(DMDFrame& f) override;

private:
	int accept_every = 1;
	int count = 0;

};