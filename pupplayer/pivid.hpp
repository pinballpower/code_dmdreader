#pragma once

#include <string>
#include <map>

#include "screen.hpp"
#include "pupplayer.hpp"

string screenToPividJSON(const map<int, PUPScreen> screens);

class PividPUPPlayer : public PUPPlayer {

public:

private:
	string basedirResized="resized";

	virtual bool startVideoPlayback(string filename, PUPScreen& screen, bool loop) override;
	virtual bool stopVideoPlayback(PUPScreen& screen, bool waitUntilStopped = true) override;
	virtual bool initializeScreens() override;

	const string resizedName(string filename, const PUPScreen& screen);
};

