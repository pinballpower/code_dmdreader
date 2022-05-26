#pragma once

#include <vector>
#include <queue>
#include <thread>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "../drm/videoplayer.hpp"
#include "../drm/videofileloader.hpp"
#include "../drm/drmframebuffer.hpp"
#include "../services/service.hpp"

#include "pupplayer.hpp"
#include "trigger.hpp"
#include "screen.hpp"
#include "playlist.hpp"

using namespace std;
using namespace boost::interprocess;

class InternalPUPPlayer : public PUPPlayer, VideoPlayerNotify {

public:
	InternalPUPPlayer(int screenNumber = 0);
	~InternalPUPPlayer();

	void playerHasFinished(int screenId);

	virtual void playbackFinished(int playerId,VideoPlayerFinishCode finishCode) override;


private:
	map<int, std::unique_ptr<VideoPlayer>> videoPlayers;    // maps screen ID to a video player
	map<int, std::unique_ptr<DRMFrameBuffer>> frameBuffers; // maps screen ID to a video player
	VideoFileLoader videoFileLoader;

	bool usePreloader = false;

	string lastTrigger = "";
	string basedir; 

	bool initVideoScreen(int screenId, int displayNumber, int& planeIndex);
	bool initFrameBufferScreen(int screenId, int displayNumber, int& planeIndex);

	virtual bool updatePlayerState() override;
	virtual bool startVideoPlayback(string filename, const PUPScreen& screen, bool loop) override;
	virtual bool stopVideoPlayback(const PUPScreen& screen, bool waitUntilStopped = true) override;

	void addFinishNotify(int screenId);

	virtual bool initializeScreens() override;
};
