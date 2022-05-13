#pragma once

#include <vector>

#include "../drm/videoplayer.hpp"
#include "../services/service.hpp"

#include "trigger.hpp"
#include "screen.hpp"
#include "playlist.hpp"

class PUPPlayer : public Service {

public:
	PUPPlayer(int screenNumber = 0);
	~PUPPlayer();

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;
	bool start() override;

	void playEvent(int event);

private:
	vector <PUPTrigger> triggers;
	vector <PUPScreen> screens;
	vector <PUPPlaylist> playlists;
	vector <std::shared_ptr<VideoPlayer>> players;
};


// Helper functions to read .pup files
vector<string> splitLine(const string& line);
int parseInteger(const string &s, int defaultValue = -1);
int parseBool(const string& s, bool defaultValue = true);