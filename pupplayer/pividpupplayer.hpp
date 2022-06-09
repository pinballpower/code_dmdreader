#pragma once

#include <string>
#include <map>

#include "screen.hpp"
#include "pupplayer.hpp"
#include "pivid.hpp"


class PividPUPPlayer : public PUPPlayer {

public:

	PividPUPPlayer();

private:
	string basedirResized;
	string ffmpegOptions;
	PIVID pivid;
	vector<string> videoFiles;

	virtual bool startVideoPlayback(string filename, PUPScreen& screen, bool loop) override;
	virtual bool stopVideoPlayback(PUPScreen& screen, bool waitUntilStopped = true) override;
	virtual bool initializeScreens() override;

	const string resizedName(string filename, const PUPScreen& screen);
	const json exportAsJSON();
	const json exportScreenAsJSON(PUPScreen& screen);
	void updatePIVID();
};

