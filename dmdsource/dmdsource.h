#pragma once

#include <boost/property_tree/ptree.hpp>

#include "../dmd/dmdframe.h"

using namespace std;

class SourceProperties {

public:
	int width;
	int height;
	int bitsperpixel;

	SourceProperties(int width = 0, int height = 0, int bitsperpixel = 0);
	SourceProperties(DMDFrame &f);

};

class DMDSource {

public:
	virtual DMDFrame next_frame(bool blocking = true);
	virtual bool finished();
	virtual bool frame_ready();
	virtual void close();
	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);

	virtual SourceProperties get_properties();

	virtual int dropped_frames();
};
