#include "pivid.hpp"

#include <filesystem>
#include <string>

#include <boost/log/trivial.hpp>


void resizeFile(string filename, string newName) {
	
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
			resizeFile(f, resizedName(f, screen.second));
		}
	}
	return true;
}

const string PividPUPPlayer::resizedName(string filename, const PUPScreen &screen) {
	int width = screen.composition.width;
	int height = screen.composition.height;
	std::filesystem::path p = filename;
	string ext = p.extension();
	string basename = p.replace_extension();
	string newName = basename + "-" + to_string(width) + "x" + to_string(height) + ext;
	return newName;
}


