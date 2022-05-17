#pragma once

#include <thread>
#include <string>
#include <map>
#include <queue>

#include "videofile.hpp"

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

class VideoFileLoader {

public:

	VideoFileLoader();
	~VideoFileLoader();

	unique_ptr<VideoFile> getFile(const string filename);
	void preloadFile(string filename);

private:

	void loaderLoop();

	bool finished = false;
	queue<string> filesToLoad;
	map<string, unique_ptr<VideoFile>> preloadedFiles;
	thread loaderThread;
	boost::interprocess::interprocess_semaphore semaphoreLoadNextFile = boost::interprocess::interprocess_semaphore(0);

};