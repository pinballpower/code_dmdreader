#pragma once

#include <vector>
#include <queue>
#include <thread>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "../drm/videoplayer.hpp"
#include "../services/service.hpp"

#include "trigger.hpp"
#include "screen.hpp"
#include "playlist.hpp"

using namespace std;
using namespace boost::interprocess;

const string QUIT = "quit";

class PlayerState {

public:
	int priority=-1;
	bool playing = false;
};

class PUPPlayer : public Service {

public:
	PUPPlayer(int screenNumber = 0);
	~PUPPlayer();

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;
	virtual bool start() override;
	virtual void stop() override;
	virtual string name() override;

	virtual std::pair<ServiceResponse, string> command(const string& cmd);


private:
	map<string,PUPTrigger> triggers; // map trigger to trigger data
	vector<PUPScreen> screens;
	map <string, PUPPlaylist> playlists;
	queue<string> eventsToProcess;
	map<int, std::unique_ptr<VideoPlayer>> players; // maps screen ID to a video player
	map<int, PlayerState> playerStates;				// keeps track of the state of all players

	string lastTrigger = "";
	string basedir; 

	void eventLoop();
	thread eventThread;
	interprocess_semaphore eventReady = interprocess_semaphore(0);

	bool initScreens(string screensToConfigure, int displayNumber);

	void sendEvent(const string event);
	void processTrigger(string trigger);
	void updatePlayerState();
};


// Helper functions to read .pup files
vector<string> splitLine(const string& line);
int parseInteger(const string &s, int defaultValue = -1);
int parseBool(const string& s, bool defaultValue = true);