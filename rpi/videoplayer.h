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

private:
	string filename;
	bool playing;

	bool playLoop(int loopCount=0);

	thread playerThread;
};
