#include "pupplayer.hpp"

#include <queue>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/foreach.hpp>


#include "../drm/drmhelper.hpp"
#include "../drm/videofile.hpp"
#include "trigger.hpp"

int parseInteger(const string& s, int defaultValue) {
	if (s == "") {
		return defaultValue;
	}
	return std::stoi(s);
}

float parseFloat(const string& s, float defaultValue) {
	if (s == "") {
		return defaultValue;
	}
	return std::stof(s);
}

bool parseBool(const string& s, bool defaultValue)
{
	if (s == "") {
		return defaultValue;
	}
	if ((s == "0") || (s == "off")) {
		return false;
	}
	if ((s == "1") || (s == "on")) {
		return true;
	}
	return defaultValue;
}

enum class CSVState {
	UnquotedField,
	QuotedField,
	QuotedQuote
};

vector<string> splitLine(const string& line) {
	CSVState state = CSVState::UnquotedField;
	std::vector<std::string> fields{ "" };
	size_t i = 0; // index of the current field
	for (char c : line) {
		switch (state) {
		case CSVState::UnquotedField:
			switch (c) {
			case ',': // end of field
				fields.push_back(""); i++;
				break;
			case '"': state = CSVState::QuotedField;
				break;
			default:  fields[i].push_back(c);
				break;
			}
			break;
		case CSVState::QuotedField:
			switch (c) {
			case '"': state = CSVState::QuotedQuote;
				break;
			default:  fields[i].push_back(c);
				break;
			}
			break;
		case CSVState::QuotedQuote:
			switch (c) {
			case ',': // , after closing quote
				fields.push_back(""); i++;
				state = CSVState::UnquotedField;
				break;
			case '"': // "" -> "
				fields[i].push_back('"');
				state = CSVState::QuotedField;
				break;
			default:  // end of quote
				state = CSVState::UnquotedField;
				break;
			}
			break;
		}
	}

	return fields;
}

template <class myType>
std::optional<vector<myType>> readConfigFile(string filename) {
	vector<myType> result;

	ifstream is;
	is.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		is.open(filename);
	}
	catch (std::ios_base::failure e) {
		return  std::nullopt;
	}

	bool eof = false;
	string line;
	// ignore first line with the field headers
	std::getline(is, line);
	while (!eof) {
		try {
			if (!std::getline(is, line)) {
				if (std::cin.eof()) {
					eof = true;
				}
				else {
					throw std::ios_base::failure("couldn't read file");
				}
			}
			boost::trim_right(line);

			myType obj = myType(line);
			if (obj.isValid()) {
				result.push_back(obj);
			}
		}
		catch (std::ios_base::failure e) {
			eof = true;
		}
	}
	return result;
}

PUPPlayer::~PUPPlayer()
{
	stop();
}

void PUPPlayer::addFinishNotify(int screenId) {
	players[screenId]->setNotify(this);
}

void PUPPlayer::playDefaultVideo(int screenId) {
	if (players.contains(screenId)) {
		PUPScreen screen = screens[screenId];
		if (screen.playFile != "") {
			string playfile = basedir + "/" + screen.playList + "/" + screen.playFile;

			if (PUPPlayer::hasSupportedExtension(playfile)) {
				players[screen.screenNum]->startPlayback(make_unique<VideoFile>(playfile, true), true);
				BOOST_LOG_TRIVIAL(info) << "[pupplayer] looping default video " << playfile << " on screen " << screenId;
			}
		}
	}
	else {
		BOOST_LOG_TRIVIAL(error) << "[pupplayer] ooops, playDefaultVideo, screen " << screenId << " doesn't exist";
	}
}

bool PUPPlayer::initVideoScreen(int screenId, int displayNumber, int& planeIndex) {

	CompositionGeometry fullscreen = DRMHelper::getFullscreenResolution(displayNumber);

	auto screen = screens[screenId];

	CompositionGeometry composition;
	composition.x = screen.x * fullscreen.width;
	composition.y = screen.y * fullscreen.height;
	composition.width = screen.width * fullscreen.width ;
	composition.height = screen.height * fullscreen.height;

	players[screenId] = std::make_unique<VideoPlayer>(displayNumber, planeIndex, composition, screenId);
	addFinishNotify(screenId);
	playerStates[screenId] = PlayerState();
	planeIndex++;

	return true;
}

bool PUPPlayer::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	string filename;

	int displayNumber = pt_source.get("display_number", 0);
	basedir = pt_source.get("directory", "");


	{
		filename = basedir + "/screens.pup";
		auto screens = readConfigFile<PUPScreen>(filename);
		if (!screens) {
			BOOST_LOG_TRIVIAL(error) << "[pupplayer] can't read file " << filename;
			return false;
		}
		// move to a map
		for (PUPScreen screen : screens.value()) {
			this->screens[screen.screenNum] = screen;
		}

		int planeIndex = 0;
		BOOST_FOREACH(const boost::property_tree::ptree::value_type & v, pt_source.get_child("screens")) {
			int screenNum = v.second.get("screenNum", -1);
			string screenType = v.second.get("type", "unknown");
			// TODO: set layers

			bool found = false;
			for (auto screen : this->screens) {
				if (screen.second.screenNum == screenNum) {
					found = true;
					if (screenType == "video") {
						initVideoScreen(screenNum, displayNumber, planeIndex);
					}
					break;
				}
			}
			if (!found) {
				BOOST_LOG_TRIVIAL(error) << "[pupplayer] couldn't find screen " << screenNum;
			}
		}

	}

	{
		filename = basedir + "/triggers.pup";
		auto triggerList = readConfigFile<PUPTrigger>(filename);
		if (!triggerList) {
			BOOST_LOG_TRIVIAL(error) << "[pupplayer] couldn't read file " << filename;
			return false;
		}
		// move to a map
		for (auto t : triggerList.value()) {
			triggers[t.trigger] = t;
		}
	}

	{
		filename = basedir + "/playlists.pup";
		auto playlists = readConfigFile<PUPPlaylist>(filename);
		if (!playlists) {
			BOOST_LOG_TRIVIAL(error) << "[pupplayer] can't read file " << filename;
			return false;
		}
		// move to a map and scan files
		for (auto pl : playlists.value()) {
			this->playlists[pl.folder] = pl;
			this->playlists[pl.folder].scanFiles(basedir);
		}
	}

	BOOST_LOG_TRIVIAL(info) << "[pupplayer] read " << this->triggers.size() << " triggers, "
		<< this->screens.size() << " screens and "
		<< this->playlists.size() << " playlists";

	// start default videos
	for (auto &player: this->players) {
		playDefaultVideo(player.first);
	}

	return true;
}

void PUPPlayer::playerHasFinished(int screenId) {
	BOOST_LOG_TRIVIAL(info) << "[pupplayer] playback on screen " << screenId << " has been finished";
}

bool PUPPlayer::start()
{
	eventThread = thread(&PUPPlayer::eventLoop, this);
	return true;
}

void PUPPlayer::stop()
{
	// send quit signal and wait until all events have been processed
	sendEvent(QUIT);
	if (eventThread.joinable()) {
		eventThread.join();
	}
}

string PUPPlayer::name()
{
	return "pupplayer";
}

std::pair<ServiceResponse, string> PUPPlayer::command(const string& cmd)
{
	if (cmd.starts_with("trigger:")) {
		eventsToProcess.push(cmd);
		eventReady.post();
		return std::pair<ServiceResponse, string>(ServiceResponse::OK, "queued");
	}
	else {
		return std::pair<ServiceResponse, string>(ServiceResponse::ERROR, "");
	}
}

bool PUPPlayer::hasSupportedExtension(string filename)
{
	for (const string suffix : PUPPLAYER_SUPPORTED_EXTENSIONS) {
		if (filename.ends_with(suffix)) {
			return true;
		}
	}
	return false;
}

void PUPPlayer::playbackFinished(int playerId, VideoPlayerFinishCode finishCode)
{
	BOOST_LOG_TRIVIAL(warning) << "[pupplayer] video player " << playerId << " finished playback";
	playerStates[playerId].playing = false;
	if (finishCode != VideoPlayerFinishCode::STOPPED_FOR_NEXT_VIDEO) {
//		playDefaultVideo(playerId);
	}
}

void PUPPlayer::eventLoop()
{
	bool finished = false;
	while (!finished) {
		eventReady.wait();
		string event = eventsToProcess.front();
		eventsToProcess.pop();

		if (event == QUIT) {
			finished = true;
		}
		else if (event.starts_with("trigger:")) {
			processTrigger(event.substr(8));
		}
		else if (event.starts_with("video:")) {
			vector<string>parts = splitLine(event);
			if (parts.size() == 4) {
				string filename = parts[1];
				int screenNumber = parseInteger(parts[2], -1);
				bool loop = parseBool(parts[3], false);
				if (screenNumber >= 0) {
					startVideoPlayback(filename, screenNumber, true);
				}
			}
			
		}
		else {
			BOOST_LOG_TRIVIAL(warning) << "[pupplayer] unknown event " << event;
		}
	}
}

void PUPPlayer::sendEvent(const string event)
{
	eventsToProcess.push(event);
	eventReady.post();
}


void PUPPlayer::updatePlayerState() {
	for (auto& playerState : playerStates) {
		int playerId = playerState.first;
		auto& player = players[playerId];
		if (!player) {
			BOOST_LOG_TRIVIAL(warning) << "[pupplayer] got a null player, something is terribly wrong :( ";
			continue;
		}
		if (!player->isPlaying()) {
			playerState.second.playing = false;
			playerState.second.priority = -1;
		}
	}
}

void PUPPlayer::processTrigger(string trigger)
{
	if (trigger == lastTrigger) {
		BOOST_LOG_TRIVIAL(trace) << "[pupplayer] ignoring duplicate trigger " << trigger;
		return;
	}
	else {
		lastTrigger = trigger;
	}

	if (!triggers.contains(trigger)) {
		BOOST_LOG_TRIVIAL(error) << "[pupplayer] trigger " << trigger << " unknown, ignoring";
		return;
	}

	const auto& triggerData = triggers[trigger];

	if (!playerStates.contains(triggerData.screennum)) {
		BOOST_LOG_TRIVIAL(trace) << "[pupplayer] trigger " << trigger << " for inactive screen, ignoring";
		return;
	}

	string playfile = triggerData.playfile;
	if (playfile == "") {
		PUPPlaylist pl = playlists[triggerData.playlist];
		playfile = pl.nextFile();
	}
	else {
		playfile = basedir + "/" + triggerData.playlist + "/" + playfile;
	}

	updatePlayerState();

	PlayerState matchingPlayerState = playerStates[triggerData.screennum];

	// Handle priorities
	if ((triggerData.priority < matchingPlayerState.priority) ||
		(triggerData.priority == matchingPlayerState.priority) && (triggerData.loop == TriggerLoop::SKIP_SAME_PRIORITY))
	{
		BOOST_LOG_TRIVIAL(debug) << "[pupplayer] ignoring trigger " << trigger << " with priority " << triggerData.priority
			<< " as a player with priority " << matchingPlayerState.priority << " is still running on screen " << triggerData.screennum;
		return;
	}

	// When stopping make sure that the player is really stopped before going on with the next events
	if (triggerData.loop == TriggerLoop::STOP_FILE) {
		players[triggerData.screennum]->stop();
		while (players[triggerData.screennum]->isPlaying()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
		}
		BOOST_LOG_TRIVIAL(debug) << "[pupplayer] trigger " << trigger << " stopped playback on screen  " << triggerData.screennum;
		return;
	}

	// TODO: Handle more trigger loop types
	//  SPLASH_RESUME,SET_BACKGROUND

	bool loop = false;
	// TODO: What's the difference between "loop" and "loopFile"?
	if ((triggerData.loop == TriggerLoop::LOOP) || (triggerData.loop == TriggerLoop::LOOP_FILE)) {
		loop = true;

	}

	if (playfile == "") {
		BOOST_LOG_TRIVIAL(debug) << "[pupplayer] trigger " << trigger << " no playfile, doing nothing ";
		return;
	}

	playerStates[triggerData.screennum].priority = triggerData.priority;

	startVideoPlayback(playfile, triggerData.screennum, loop);

}

void PUPPlayer::startVideoPlayback(string filename, int screenNumber, bool loop) {
	// TODO: Preload video files 

	auto& player = players[screenNumber];
	if (!player) {
		BOOST_LOG_TRIVIAL(warning) << "[pupplayer] got a null player, something is terribly wrong :( ";
		return;
	}

	if (PUPPlayer::hasSupportedExtension(filename)) {
		player->startPlayback(make_unique<VideoFile>(filename, true), loop);
		playerStates[screenNumber].playing = true;
		BOOST_LOG_TRIVIAL(info) << "[pupplayer] play file " << filename << " on screen " << screenNumber;
	}
	else {
		BOOST_LOG_TRIVIAL(debug) << "[pupplayer] ignoring play file " << filename << ", unsupported extension";
	}
}

PUPPlayer::PUPPlayer(int screenNumber)
{
}

