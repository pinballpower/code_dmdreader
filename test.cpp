#include "../util/glob.hpp"

VideoPlayer vp1 = VideoPlayer(screenNumber, 1, CompositionGeometry(100, 100, 700, 300));
VideoPlayer vp2 = VideoPlayer(screenNumber, 0, CompositionGeometry());

queue<unique_ptr<VideoFile>> files;
for (auto& p : glob::glob("/home/matuschd/code_dmdreader/samples/spagb_100b2s/*/*.mp4")) {
	auto v = std::make_unique<VideoFile>(p);
	v->parseStreams(); // pre-parse
	files.push(std::move(v));
	BOOST_LOG_TRIVIAL(error) << "opened " << p;
}


videoPlayer.openScreen();
while (files.size() > 0) {
	vp1.startPlayback(std::move(files.front()), true);
	files.pop();
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	vp2.startPlayback(std::move(files.front()), true);
	files.pop();
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}
