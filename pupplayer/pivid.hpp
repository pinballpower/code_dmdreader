#pragma once

#include <string>
#include <map>

#include "screen.hpp"
#include "pupplayer.hpp"

string screenToPividJSON(const map<int, PUPScreen> screens);

class PividPUPPlayer : public PUPPlayer {

public:

private:
	string basedirResized = "/tmp";

	virtual bool startVideoPlayback(string filename, const PUPScreen& screen, bool loop) override;
	virtual bool stopVideoPlayback(const PUPScreen& screen, bool waitUntilStopped = true) override;
	virtual bool initializeScreens() override;

	const string resizedName(string filename, const CompositionGeometry& geometry);
};

