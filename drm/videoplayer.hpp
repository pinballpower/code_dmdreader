#pragma once

#include <string>
#include <thread>

#include "drmprimeout.hpp"


using namespace std;

class VideoPlayer {

public: 

	VideoPlayer(int screenNumber=0, int planeNumber=0);

	// open/close connection to DRM
	bool openScreen();
	void closeScreen();

	// Playback
	void play(string filename, int loopCount=0);
	void playBackground(string filename, int loopCount=0);
	bool isPlaying();
	void stop();

	void setScaling(int x, int y, int width, int height);

private:

	// scaling (-1 means full-screen)
	int x = -1;
	int y = -1;
	int width = -1;
	int height = -1;

	int screenNumber = 0;
	int planeNumber = 0;

	bool playing = false;
	bool screenOpened = false;

	bool playLoop(string filename, int loopCount=0);

	thread playerThread;

	// screen
	DRMPrimeOut* dpo = nullptr;
};
