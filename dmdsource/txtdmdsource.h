#pragma once

#include <fstream>

#include "../dmd/dmdframe.h"
#include "dmdsource.h"

class TXTDMDSource : public DMDSource {

public:

	TXTDMDSource();
	TXTDMDSource(string filename);
	~TXTDMDSource();


	virtual DMDFrame next_frame(bool blocking = true);

	virtual bool finished();
	virtual bool frame_ready();

	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);

	virtual SourceProperties get_properties();

private:

	DMDFrame preloaded_frame;
	ifstream is;

	void read_next_frame();
	bool open_file(string filename);

	int bits = 0;
	bool eof = false;
};