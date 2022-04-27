#pragma once

#include <fstream>

#include "../dmd/dmdframe.h"
#include "dmdsource.h"

class TXTDMDSource : public DMDSource {

public:

	TXTDMDSource();
	TXTDMDSource(string filename);
	~TXTDMDSource();


	virtual DMDFrame getNextFrame(bool blocking = true) override;

	virtual bool isFinished() override;
	virtual bool isFrameReady() override;

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

	virtual SourceProperties getProperties() override;

private:

	DMDFrame preloaded_frame;
	ifstream is;

	void preloadNextFrame();
	bool openFile(string filename);

	int bits = 0;
	bool eof = false;
};