#pragma once

#include <queue>

#include "../dmd/dmdframe.h"
#include "dmdsource.h"

class DATDMDSource : DMDSource {

public:

	DATDMDSource();
	DATDMDSource(string filename);
	~DATDMDSource();

	bool read_file(string filename);

	virtual DMDFrame* next_frame(bool blocking = true);

	virtual bool finished();
	virtual bool frame_ready();

	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);

	virtual void get_properties(SourceProperties* p);

private:

	queue<DMDFrame*> frames;
};