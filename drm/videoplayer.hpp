#pragma once

#include <string>
#include <thread>

#include "drmprimeout.hpp"
#include "drmhelper.hpp"
#include "videofile.hpp"

#include <boost/log/trivial.hpp>

using namespace std;

class VideoPlayerNotify {

public: 
	virtual void playbackFinished(int playerId) {};
};

class VideoPlayer {

public: 

	VideoPlayer(int screenNumber=0, int planeNumber=0, CompositionGeometry composition = CompositionGeometry(), int playerId=0);
	VideoPlayer(const VideoPlayer&) = delete;
	VideoPlayer(VideoPlayer&&) = default;
	~VideoPlayer();

	// open/close connection to DRM
	bool openScreen();
	void closeScreen();

	// Playback
	void startPlayback(unique_ptr<VideoFile> videoFile, bool loop = false);
	bool isPlaying();
	void stop();
	void pause(bool paused=true);
	void setNotify(VideoPlayerNotify* videoPlayerNotify);

	CompositionGeometry getCompositionGeometry() const;

private:
	CompositionGeometry compositionGeometry;

	int playerId = 0;
	int screenNumber = 0;
	int planeNumber = 0;

	bool terminate = false;
	bool playing = false;
	bool screenOpened = false;
	bool paused = false;
	bool transparentWhenStopped = true;

	void playLoop(bool loop);

	unique_ptr<VideoFile> currentVideo = unique_ptr<VideoFile>(nullptr);

	VideoPlayerNotify* videoPlayerNotify = nullptr;

	thread playerThread;

	// screen
	DRMPrimeOut* dpo = nullptr;
};
