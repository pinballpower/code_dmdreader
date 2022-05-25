
#include "videofileloader.hpp"

#include <boost/log/trivial.hpp>

VideoFileLoader::VideoFileLoader()
{
	loaderThread = thread(&VideoFileLoader::loaderLoop, this);
}

VideoFileLoader::~VideoFileLoader()
{
	finished = false;
	semaphoreLoadNextFile.post();
	if (loaderThread.joinable()) {
		loaderThread.join();
	}
}

unique_ptr<VideoFile> VideoFileLoader::getFile(const string filename)
{
	// get and remove it from preloaded map
	auto preloaded = preloadedFiles.extract(filename);

	// and start preload again
	preloadFile(filename);
	
	if (not preloaded.empty()) {
		return std::move(preloaded.mapped());
	}
	else {
		// file was not pre-loaded
		unique_ptr<VideoFile> res = make_unique<VideoFile>(filename, true);
		return std::move(res);
	}
}

void VideoFileLoader::preloadFile(string filename)
{
	filesToLoad.push(filename);
	semaphoreLoadNextFile.post();
}

void VideoFileLoader::loaderLoop()
{
	while (true) {

		semaphoreLoadNextFile.wait();
		if (finished) {
			break;
		}

		if (filesToLoad.size() < 1) {
			BOOST_LOG_TRIVIAL(error) << "[videofileloader] ooops, queue is empty, this should not happen";
		}
		else {
			auto filename = filesToLoad.front();
			filesToLoad.pop();
			if (! preloadedFiles.contains(filename)) {
				preloadedFiles[filename] = std::make_unique<VideoFile>(filename, true);
			}
		}
	}
}
