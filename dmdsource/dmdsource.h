#pragma once

#include <boost/property_tree/ptree.hpp>

#include "../dmd/dmdframe.h"

using namespace std;

typedef struct SourceProperties {
	int width;
	int height;
	int bitsperpixel;
} SourceProperties;

class DMDSource {

public:
	virtual unique_ptr<DMDFrame> next_frame(bool blocking = true);
	virtual bool finished();
	virtual bool frame_ready();
	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);

	virtual void get_properties(SourceProperties* p);
};
