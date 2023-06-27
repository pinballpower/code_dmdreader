#pragma once

#include <fstream>

#include "../dmd/dmdframe.hpp"
#include "dmdsource.hpp"

class TXTDMDSource : public DMDSource {

public:

	TXTDMDSource();
	TXTDMDSource(const string& filename);
	~TXTDMDSource();


	virtual DMDFrame getNextFrame() override;

	virtual bool isFinished() override;
	virtual bool isFrameReady() override;

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

	virtual SourceProperties getProperties() override;

private:

	DMDFrame currentFrame;
	DMDFrame preloadedFrame;
	uint32_t preloadedFrameTimestamp = -1;
	ifstream is;
	bool useTimingData = true;
	unsigned long startMillisec = 0;
	unsigned long lastFrameSentMillis = 0;
	unsigned int frameEveryMs = 0;

	void preloadNextFrame();
	bool openFile(const string& filename);
	uint32_t getCurrentTimestamp();

	int bits = 0;
	bool eof = false;

	int id = 0;
};