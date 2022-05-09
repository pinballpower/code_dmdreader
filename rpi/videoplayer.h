#pragma once

#include <string>

using namespace std;

class VideoPlayer {

public: 

	VideoPlayer(const string filename);

	void play();
	bool isPlaying();

private:
	string filename;
	bool playing;

	bool playLoop(int loopCount=0);

};
