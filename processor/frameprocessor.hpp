#pragma once

#include "../dmd/dmdframe.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

class DMDFrameProcessor {

public:

	DMDFrameProcessor();
	~DMDFrameProcessor();

	virtual DMDFrame processFrame(DMDFrame &f);
	virtual void close();

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);

	string name="FrameProcessor";
};