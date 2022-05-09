#pragma once

#include <string>
#include <thread>


using namespace std;

class VideoPlayer {

public: 

	VideoPlayer(const string filename);

	void play(int loopCount=0);
	void playBackground(int loopCount=0);
	bool isPlaying();
	void stop();

	void setScaling(int x, int y, int width, int height);

private:

	// scaling (-1 means full-screen)
	int x = -1;
	int y = -1;
	int width = -1;
	int height = -1;

	string filename;
	bool playing;

	bool playLoop(int loopCount=0);

	thread playerThread;
};
