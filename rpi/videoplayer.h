#pragma once

#include <string>
#include <thread>


using namespace std;

class VideoPlayer {

public: 

	VideoPlayer(const string filename);

	void play();
	void playBackground();
	bool isPlaying();

private:
	string filename;
	bool playing;

	bool playLoop(int loopCount=0);

	thread playerThread;
};
