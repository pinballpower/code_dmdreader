#pragma once

#include <vector>

#include "../drm/videoplayer.hpp"
#include "../services/service.hpp"

#include "trigger.hpp"
#include "screen.hpp"

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
	vector <std::shared_ptr<VideoPlayer>> players;
};


// Helper functions to read .pup files
vector<string> splitLine(const string& line);
int flex_stoi(const string &s, int defaultValue = -1);