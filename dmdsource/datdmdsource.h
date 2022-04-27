#pragma once

#include <queue>

#include "../dmd/dmdframe.h"
#include "dmdsource.h"

class DATDMDSource : public DMDSource {

public:

	DATDMDSource();
	DATDMDSource(string filename);
	~DATDMDSource();

	bool readFile(string filename);

	virtual DMDFrame getNextFrame(bool blocking = true) override;

	virtual bool isFinished() override;
	virtual bool isFrameReady() override;

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

	virtual SourceProperties getProperties() override;

private:

	DMDFrame readFromDatFile(std::ifstream& fis);
	queue<DMDFrame> frames;
};