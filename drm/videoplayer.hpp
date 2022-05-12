#pragma once

#include <string>
#include <thread>

#include "drmprimeout.hpp"
#include "drmhelper.hpp"


using namespace std;

class VideoPlayer {

public: 

	VideoPlayer(int screenNumber=0, int planeNumber=0, CompositionGeometry composition = CompositionGeometry());

	// open/close connection to DRM
	bool openScreen();
	void closeScreen();

	// Playback
	void startPlayback(string filename, bool loop = false);
	bool isPlaying();
	void stop();
	void pause(bool paused=true);

	void setComposition(CompositionGeometry compositionGeometry);

private:

	// scaling (-1 means full-screen)
	CompositionGeometry compositionGeometry;

	int screenNumber = 0;
	int planeNumber = 0;

	bool terminate = false;
	bool playing = false;
	bool screenOpened = false;
	bool paused = false;

	bool playLoop(string filename, bool loop);

	thread playerThread;

	// screen
	DRMPrimeOut* dpo = nullptr;
};
