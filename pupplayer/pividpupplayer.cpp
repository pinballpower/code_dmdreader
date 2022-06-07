#include "pividpupplayer.hpp"
#include "pivid.hpp"

#include <filesystem>
#include <string>
#include <cstdlib>

#include <boost/log/trivial.hpp>

int roundup16(int i) {
	return ((i + 15) / 16) * 16;
}

bool resizeFile(string filename, string newName, const PUPScreen& screen) 
{
	int width = roundup16(screen.composition.width);
	int height = roundup16(screen.composition.height);

	if (std::filesystem::exists(newName)) {
		BOOST_LOG_TRIVIAL(info) << "[pivid] " << newName << " already exists";
		return false;
	}
	string command = "ffmpeg -i " + filename + " -s " + to_string(width) + "x" + to_string(height) + " -an " + newName;

	std::filesystem::path p = std::filesystem::path(newName);
	p.remove_filename();
	std::filesystem::create_directories(p);
	if (! std::filesystem::exists(p)) {
		BOOST_LOG_TRIVIAL(info) << "[pivid] couldn't create " << p;
	}

	BOOST_LOG_TRIVIAL(info) << "[pivid] converting " << filename << " to " << newName << " using " << command;
	return (std::system(command.c_str()) == 0);
}

string screenToPividJSON(const map<int, PUPScreen> screens)
{
	return "";
}

bool PividPUPPlayer::startVideoPlayback(string filename, PUPScreen& screen, bool loop)
{
	return false;
}

bool PividPUPPlayer::stopVideoPlayback(PUPScreen& screen, bool waitUntilStopped)
{
	return false;
}

bool PividPUPPlayer::initializeScreens()
{
	// prepare videos
	for (auto screen : screens) {
		vector<string> files = getFilesForScreen(screen.second.screenNum);
		for (auto f : files) {
			resizeFile(f, resizedName(f, screen.second), screen.second);
		}
	}

	// start PIVID
	pivid.startServer(std::filesystem::current_path().generic_string());

	exportJSON();

	return true;
}

const string PividPUPPlayer::resizedName(string filename, const PUPScreen& screen) {
	int width = roundup16(screen.composition.width);
	int height = roundup16(screen.composition.height);
	std::filesystem::path p = filename;
	string ext = p.extension();
	string basename = p.replace_extension();

	// remove original base directory
	if (basename.starts_with(basedir)) {
		basename = basename.substr(basedir.length());
	}

	string newName = basedirResized + "/" + basename + "-" + to_string(width) + "x" + to_string(height) + ".mp4";
	return newName;
}

const json PividPUPPlayer::exportAsJSON() {

	json result;
	result["screens"]["HDMI-1"]["mode"] = { 1920,1080,60 };
	result["screens"]["HDMI-1"]["update_hz"] = 30;
	result["screens"]["HDMI-1"]["layers"] = json::array();
	auto& layers = result["screens"]["HDMI-1"]["layers"];
	for (auto& s : screens) {
		auto& screen = s.second;
		if (screen.currentFile != "") {
			layers.push_back(exportScreenAsJSON(screen));
		}
	}
	return result;
}

const json PividPUPPlayer::exportScreenAsJSON(PUPScreen& screen) {
	//string result = "{"
	//	"\"media\": \"" + screen.currentFile + "\"," +
	//	"\"play\" : { \"t\": [ 0,3 ], \"v\": [ 0,3 ], \"repeat\": true }," +
	//	"\"to_xy\" : [" + std::to_string(screen.composition.x) + "," + std::to_string(screen.composition.y) + "] }";

	string realName = "";
	float duration = 0;
	if (screen.currentFile != "") {
		realName = resizedName(screen.currentFile, screen);
		duration = pivid.getDuration(realName);
	}
	json result;
	result["media"] = realName;
	result["play"]["t"] = { 0, duration };
	result["play"]["v"] = { 0, duration };
	result["play"]["repeat"] = true;
	result["to_xy"] = { screen.composition.x , screen.composition.y };

	return result;

}


