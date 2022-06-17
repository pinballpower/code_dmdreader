#include "pividpupplayer.hpp"
#include "pivid.hpp"

#include <filesystem>
#include <string>
#include <cstdlib>
#include <chrono>

#include <boost/log/trivial.hpp>
#include <boost/foreach.hpp>

int roundup16(int i) {
	return ((i + 15) / 16) * 16;
}

bool resizeFile(string filename, string newName, const PUPScreen& screen, string ffmpegOptions) 
{
	int width = roundup16(screen.composition.width);
	int height = roundup16(screen.composition.height);

	if (std::filesystem::exists(newName)) {
		BOOST_LOG_TRIVIAL(info) << "[pivid] " << newName << " already exists";
		return true;
	}

	string command;
	if (filename.ends_with(".png")) {
		command = "cp \"" + filename + "\"" + newName;
	}
	else {
		command = "ffmpeg -i \"" + filename + "\" -s " + to_string(width) + "x" + to_string(height) + " -an " + ffmpegOptions + " " + newName;
	}

	std::filesystem::path p = std::filesystem::path(newName);
	p.remove_filename();
	std::filesystem::create_directories(p);
	if (! std::filesystem::exists(p)) {
		BOOST_LOG_TRIVIAL(info) << "[pivid] couldn't create " << p;
	}

	BOOST_LOG_TRIVIAL(info) << "[pivid] converting " << filename << " to " << newName << " using " << command;
	std::system(command.c_str());

	return std::filesystem::exists(newName);
}


PividPUPPlayer::PividPUPPlayer()
{
	// use HVEC when resizing files
	basedirResized = "x265";
	ffmpegOptions = " -c:v libx265 ";
}

bool PividPUPPlayer::startVideoPlayback(string filename, PUPScreen& screen, bool loop)
{
	if (!loop) {
		// after the video has been finished, return to default video
		string realFilename = resizedName(filename, screen);
		float duration = pivid.getDuration(realFilename);
		int delayMilliseconds = (duration * 1000) - 500;
		std::thread threadSwitchToDefault(&PividPUPPlayer::playDefaultVideo, this, std::ref(screen), filename, delayMilliseconds);
		threadSwitchToDefault.detach();
	}

	screen.currentFile = filename;
	screen.loopCurrentFile = loop;
	screen.startTimeStampMilliseconds = 0;
	updatePIVID();
	return true;
}

bool PividPUPPlayer::stopVideoPlayback(PUPScreen& screen, bool waitUntilStopped)
{
	screen.currentFile = "";
	updatePIVID();
	return true;
}

bool PividPUPPlayer::initializeScreens()
{
	// clear video file list
	videoFiles.clear();

	// prepare videos
	for (auto screen : screens) {
		vector<string> files = getFilesForScreen(screen.second.screenNum);
		for (auto f : files) {
			auto nameOfResizedFile = resizedName(f, screen.second);
			if (resizeFile(f, nameOfResizedFile, screen.second, ffmpegOptions)) {
				if (std::find(videoFiles.begin(), videoFiles.end(), nameOfResizedFile) == videoFiles.end()) {
					videoFiles.push_back(nameOfResizedFile);
				}
			}
		}
	}

	// start PIVID
	pivid.startServer(std::filesystem::current_path().generic_string());
	updatePIVID();

	return true;
}

void PividPUPPlayer::updatePIVID() {
	auto jsonStr = exportAsJSON().dump();
	BOOST_LOG_TRIVIAL(info) << jsonStr;
	pivid.sendRequest("/play", boost::beast::http::verb::post, jsonStr);
}

const string PividPUPPlayer::resizedName(string filename, const PUPScreen& screen) {
	int width = roundup16(screen.composition.width);
	int height = roundup16(screen.composition.height);
	std::filesystem::path p = filename;
	string ext = p.extension();
	string basename = p.replace_extension();

	string extNew = ".mp4";
	if (ext == ".png") {
		extNew = ext;
	}

	// remove original base directory
	if (basename.starts_with(basedir)) {
		basename = basename.substr(basedir.length());
	}

	string newName = basedirResized + "/" + basename + "-" + to_string(width) + "x" + to_string(height) + extNew;
	std::replace(newName.begin(), newName.end(), ' ', '_'); // rmeove spaces
	return newName;
}

const json PividPUPPlayer::exportAsJSON() {

	json result;
	vector<string> files;

	result["zero_time"] = 0.0f;
	result["main_buffer_time"] = 0.2;
	result["main_loop_hz"] = 30;

	for (const auto d : displays) {

		auto name = d.first;
		if (name == "") {
			continue;
		}

		auto displayData = d.second;

		result["screens"][name]["mode"] = { displayData.width,displayData.height,displayData.hz };
		result["screens"][name]["update_hz"] = displayData.updateRate;
		result["screens"][name]["layers"] = json::array();
		auto& layers = result["screens"][name]["layers"];
		for (auto& s : screens) {
			auto& screen = s.second;
			if (screen.displayName != name) {
				continue;
			}

			if (screen.currentFile != "") {
				layers.push_back(exportScreenAsJSON(screen));
				files.push_back(resizedName(screen.currentFile, screen));
			}
		}
		
	}
	return result;

}

const json PividPUPPlayer::exportScreenAsJSON(PUPScreen& screen) {
	string realName = "";
	float duration = 0;
	if (screen.currentFile != "") {
		realName = resizedName(screen.currentFile, screen);
		duration = pivid.getDuration(realName);
	}

	long startTimeMilliseconds = screen.startTimeStampMilliseconds;
	if (startTimeMilliseconds <= 0) {
		auto now = std::chrono::system_clock::now();
		startTimeMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
		screen.startTimeStampMilliseconds = startTimeMilliseconds;
	}

	double startTime = ((double)startTimeMilliseconds) / 1000;

	json result;
	result["media"] = realName;
	result["play"]["t"] = { startTime, startTime+duration};
	result["play"]["v"] = 0;
	result["play"]["rate"] = 1.0;
	
	result["play"]["repeat"] = screen.loopCurrentFile;

	result["to_xy"] = { screen.composition.x , screen.composition.y };

	return result;

}

void PividPUPPlayer::playDefaultVideo(PUPScreen& screen, string checkPlayingFile, int delayMilliseconds) {

	if (screen.playFile == "") {
		return;
	}

	if (delayMilliseconds > 0) {
		this_thread::sleep_for(chrono::milliseconds(delayMilliseconds));
	}

	// only do this if the currently playing file is still "checkPlayingFile", otherwise ignore
	if ((checkPlayingFile != "") && (screen.currentFile != checkPlayingFile)) {
		return;
	}

	screen.currentFile = screen.playList + "/" + screen.playFile;
	screen.loopCurrentFile = true;
	screen.startTimeStampMilliseconds = 0;
	updatePIVID();
}


bool PividPUPPlayer::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	// Screen sizes
	try {
		BOOST_FOREACH(const boost::property_tree::ptree::value_type & v, pt_source.get_child("screens")) {
			PIVIDDisplayData displayData;
			auto screen = v.second;
			int screenId = screen.get("screen", 0);
			string displayName = screen.get("display_name", "");
			displayData.width = screen.get("width", 1920);
			displayData.height = screen.get("height", 1080);
			displayData.hz = screen.get("hz", 60);
			displayData.updateRate = screen.get("refresh_rate", 30);

			displays[displayName] = displayData;

		}
	}
	catch (const boost::property_tree::ptree_bad_path& e) {
		BOOST_LOG_TRIVIAL(error) << "[pividpupplayer] couldn't read screen definitions";
		return false;
	}

	return PUPPlayer::configureFromPtree(pt_general, pt_source);
}

