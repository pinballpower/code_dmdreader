#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "../dmd/dmdframe.hpp"
#include "../dmd/color.hpp"

class FrameRenderer
{
public:
	FrameRenderer();
	~FrameRenderer();

	std::string name = "";

	virtual void renderFrame(DMDFrame &f);
	virtual void close();

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer);
};