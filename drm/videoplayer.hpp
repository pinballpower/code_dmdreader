#pragma once

#include <string>
#include <thread>

#include "drmprimeout.hpp"
#include "drmhelper.hpp"
#include "videofile.hpp"


using namespace std;

class VideoPlayer {

public: 

	VideoPlayer(int screenNumber=0, int planeNumber=0, CompositionGeometry composition = CompositionGeometry());
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

	CompositionGeometry getCompositionGeometry(const CompositionGeometry compositionGeometry);

private:
	CompositionGeometry compositionGeometry;

	int screenNumber = 0;
	int planeNumber = 0;

	bool terminate = false;
	bool playing = false;
	bool screenOpened = false;
	bool paused = false;

	void playLoop(bool loop);

	unique_ptr<VideoFile> currentVideo = unique_ptr<VideoFile>(nullptr);

	thread playerThread;

	// screen
	DRMPrimeOut* dpo = nullptr;
};
