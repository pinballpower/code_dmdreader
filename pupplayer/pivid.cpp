#include "pivid.hpp"

#include <filesystem>
#include <string>

#include <boost/log/trivial.hpp>

void resizeFile(string filename, int width, int height) {
	std::filesystem::path p = filename;
	string ext = p.extension();
	string basename = p.replace_extension();
	string newName = basename + "-" + to_string(width) + "x" + to_string(height) + ext;
	BOOST_LOG_TRIVIAL(info) << "[pivid] converting " << filename << " to " << newName;
}

string screenToPividJSON(const map<int, PUPScreen> screens)
{
	return "";
}

bool PividPUPPlayer::startVideoPlayback(string filename, const PUPScreen& screen, bool loop)
{
	return false;
}

bool PividPUPPlayer::stopVideoPlayback(const PUPScreen& screen, bool waitUntilStopped)
{
	return false;
}

bool PividPUPPlayer::initializeScreens()
{
	// prepare videos
	for (auto screen : screens) {
		vector<string> files = getFilesForScreen(screen.second.screenNum);
		for (auto f : files) {
			resizeFile(f, screen.second.width, screen.second.height);
		}
	}
	return true;
}


