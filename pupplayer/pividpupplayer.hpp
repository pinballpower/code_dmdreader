#pragma once

#include <string>
#include <map>

#include "screen.hpp"
#include "pupplayer.hpp"
#include "pivid.hpp"

class PIVIDDisplayData {

public:
	int width = 1920;
	int height = 1080;
	int hz = 60;
	int updateRate = 30;
};


class PividPUPPlayer : public PUPPlayer {

public:

	PividPUPPlayer();

private:
	string basedirResized;
	string ffmpegOptions;
	PIVID pivid;
	vector<string> videoFiles;
	map<string, PIVIDDisplayData> displays;

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

	virtual bool startVideoPlayback(string filename, PUPScreen& screen, bool loop) override;
	virtual bool stopVideoPlayback(PUPScreen& screen, bool waitUntilStopped = true) override;
	void playDefaultVideo(PUPScreen& screen, string checkPlayingFile, int delayMilliseconds = 0);
	virtual bool initializeScreens() override;

	const string resizedName(string filename, const PUPScreen& screen);
	const json exportAsJSON();
	const json exportScreenAsJSON(PUPScreen& screen);
	void updatePIVID();
};

