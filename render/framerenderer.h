#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "../dmd/dmdframe.h"
#include "../dmd/color.h"

class FrameRenderer
{
public:
	FrameRenderer();
	~FrameRenderer();

	virtual void renderFrame(DMDFrame &f);
	virtual void close();

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer);
};