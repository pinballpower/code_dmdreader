#pragma once

#include <string>
#include <thread>


using namespace std;

class VideoPlayer {

public: 

	VideoPlayer();

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

	bool playing;

	bool playLoop(string filename, int loopCount=0);

	thread playerThread;
};
