#pragma once

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <queue>

#include <boost/property_tree/ptree.hpp>

#include "../dmd/dmdframe.h"
#include "frameprocessor.h"

class FrameStore : public DMDFrameProcessor {

public:

	~FrameStore();

	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);
	virtual DMDFrame process_frame(DMDFrame &f);

private:

	void write_to_file(DMDFrame& f);

	ofstream outputfile;
	bool finished = true;
	bool ignore_duplicates = true;
	bool async = false;

	/// <summary>
	/// Checksums of frames that have been seen already
	/// </summary>
	unordered_set<uint32_t> seen;

	/// <summary>
	/// Queue of frames that are not yet written
	/// </summary>
	queue<DMDFrame> frames_to_write;
 
	int frameno = 1;
};