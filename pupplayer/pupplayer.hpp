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
	vector<PUPTrigger> triggers;
	vector<PUPScreen> screens;
	vector<PUPPlaylist> playlists;
	vector<std::shared_ptr<VideoPlayer>> players;
	queue<string> eventsToProcess;

	string lastTrigger = "";

	void eventLoop();
	thread eventThread;
	interprocess_semaphore eventReady = interprocess_semaphore(0);

	void sendEvent(const string event);
	void processTrigger(string trigger);

};


// Helper functions to read .pup files
vector<string> splitLine(const string& line);
int parseInteger(const string &s, int defaultValue = -1);
int parseBool(const string& s, bool defaultValue = true);