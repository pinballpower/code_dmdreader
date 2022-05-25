#pragma once

#include <vector>
#include <queue>
#include <thread>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "../drm/videoplayer.hpp"
#include "../drm/videofileloader.hpp"
#include "../drm/drmframebuffer.hpp"
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

const vector<string> PUPPLAYER_SUPPORTED_EXTENSIONS = { ".mp4" };

class PUPPlayer : public Service, public VideoPlayerNotify {

public:
	PUPPlayer(int screenNumber = 0);
	~PUPPlayer();

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;
	virtual bool start() override;
	virtual void stop() override;
	virtual string name() override;
	void playerHasFinished(int screenId);

	virtual std::pair<ServiceResponse, string> command(const string& cmd);

	static bool hasSupportedExtension(string filename);

	virtual void playbackFinished(int playerId,VideoPlayerFinishCode finishCode) override;


private:
	map<string,PUPTrigger> triggers; // map trigger to trigger data
	map<int, PUPScreen> screens;
	map <string, PUPPlaylist> playlists;
	queue<string> eventsToProcess;
	map<int, std::unique_ptr<VideoPlayer>> videoPlayers;    // maps screen ID to a video player
	map<int, std::unique_ptr<DRMFrameBuffer>> frameBuffers; // maps screen ID to a video player
	map<int, PlayerState> playerStates;				     // keeps track of the state of all players
	VideoFileLoader videoFileLoader;

	bool usePreloader = false;

	string lastTrigger = "";
	string basedir; 

	void eventLoop();
	thread eventThread;
	interprocess_semaphore eventReady = interprocess_semaphore(0);

	bool initVideoScreen(int screenId, int displayNumber, int& planeIndex);
	bool initFrameBufferScreen(int screenId, int displayNumber, int& planeIndex);

	void sendEvent(const string event);
	void processTrigger(string trigger);
	void updatePlayerState();
	void addFinishNotify(int screenId);
	void playDefaultVideo(int screenId);
	void startVideoPlayback(string filename, int screenNumber, bool loop);
	void calculateScreenCoordinates(int screenId, int screenWidth, int screenHeight);
};


// Helper functions to read .pup files
vector<string> splitLine(const string& line);
int parseInteger(const string &s, int defaultValue = -1);
bool parseBool(const string& s, bool defaultValue = true);
float parseFloat(const string& s, float defaultValue = 0);