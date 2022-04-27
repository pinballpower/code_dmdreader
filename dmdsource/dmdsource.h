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
	virtual DMDFrame getNextFrame(bool blocking = true);
	virtual bool isFinished();
	virtual bool isFrameReady();
	virtual void close();
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);

	virtual SourceProperties getProperties();

	virtual int getDroppedFrames();
};
