#include "pivid.hpp"

#include <filesystem>
#include <string>
#include <cstdlib>

#include <boost/log/trivial.hpp>


bool resizeFile(string filename, string newName, CompositionGeometry size)
{
	if (std::filesystem::exists(newName)) {
		BOOST_LOG_TRIVIAL(info) << "[pivid] " << newName << " already exists";
		return false;
	}
	string command = "ffmpeg -i " + filename + " -s " + to_string(size.width) + "x" + to_string(size.height) + " -an " + newName;

	std::filesystem::path p = std::filesystem::path(newName);
	p.remove_filename();
	if (!std::filesystem::create_directories(p)) {
		BOOST_LOG_TRIVIAL(info) << "[pivid] couldn't create " << p;
	}

	BOOST_LOG_TRIVIAL(info) << "[pivid] converting " << filename << " to " << newName << " using " << command;
	return (std::system(command.c_str()) == 0);
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
			CompositionGeometry geometry = screen.second.composition;
			resizeFile(f, resizedName(f, geometry), geometry);
		}
	}
	return true;
}

const string PividPUPPlayer::resizedName(string filename, const CompositionGeometry& geometry) {
	int width = geometry.width;
	int height = geometry.height;
	std::filesystem::path p = filename;
	string ext = p.extension();
	string basename = p.replace_extension();

	// remove original base directory
	if (basename.starts_with(basedir)) {
		basename = basename.substr(basedir.length());
	}

	string newName = basedirResized + basename + "-" + to_string(width) + "x" + to_string(height) + ".mp4";
	return newName;
}


