#include "pupplayer.hpp"

#include <queue>
#include <filesystem>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

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

void PUPPlayer::playDefaultVideo(int screenId) {
	PUPScreen screen = screens[screenId];
	if (screen.playFile != "") {
		string playfile = basedir + "/" + screen.playList + "/" + screen.playFile;

		if (PUPPlayer::hasSupportedExtension(playfile)) {
			startVideoPlayback(playfile, screen, true);
			BOOST_LOG_TRIVIAL(info) << "[pupplayer] looping default video " << playfile << " on screen " << screenId;
		}
	}
}

bool PUPPlayer::startVideoPlayback(string filename, PUPScreen& screen, bool loop)
{
	BOOST_LOG_TRIVIAL(error) << "[pupplayer] startVideoPlayback not implemented";
	return false;
}

bool PUPPlayer::stopVideoPlayback(PUPScreen& screen, bool waitUntilStopped)
{
	BOOST_LOG_TRIVIAL(error) << "[pupplayer] stopVideoPlayback not implemented";
	return false;
}

bool PUPPlayer::initializeScreens()
{
	return false;
}

void PUPPlayer::calculateScreenCoordinates(int screenId, int screenWidth, int screenHeight) {
	int screenX = 0, screenY = 0;

	auto screen = &this->screens[screenId];
	if (screen->parentScreen >= 0) {
		BOOST_LOG_TRIVIAL(error) << "[pupplayer] can't calculate screen coordinates on " << screenId << " as it has a parent screen";
		return;
	}

	if (screen->width <= 0) {
		screen->width = 1;
	}
	if (screen->height <= 0) {
		screen->height = 1;
	}

	screen->composition.x = screen->x * screenWidth;
	screen->composition.y = screen->y * screenHeight;
	screen->composition.width = screen->width * screenWidth;
	screen->composition.height = screen->height * screenHeight;

	// now update all subscreens
	for (auto &subScreen : screens) {
		if (subScreen.second.parentScreen == screenId) {

			if (subScreen.second.width <= 0) {
				subScreen.second.width = 1;
			}
			if (subScreen.second.height <= 0) {
				subScreen.second.height = 1;
			}
			subScreen.second.composition.x = subScreen.second.x * screen->composition.width + screen->composition.x;
			subScreen.second.composition.y = subScreen.second.y * screen->composition.height + screen->composition.y;
			subScreen.second.composition.width = subScreen.second.width * screen->composition.width;
			subScreen.second.composition.height = subScreen.second.height * screen->composition.height;
			subScreen.second.displayNumber = screen->displayNumber;

			BOOST_LOG_TRIVIAL(error) << "[pupplayer] " << subScreen.second.composition.width << "x" <<
				subScreen.second.composition.height << "@" <<
				subScreen.second.composition.x << "," <<
				subScreen.second.composition.y;
		}
	}
}

bool PUPPlayer::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	string filename;

	int displayNumber = pt_source.get("display_number", 0);
	basedir = pt_source.get("directory", "");

	// screens
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
	}

	// triggers
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

	// playlists
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

	// Screen to display mapping
	{
		try {
			BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, pt_source.get_child("screens")) {
				auto screen = v.second;
				int screenId = screen.get("screen", 0);
				int displayId = screen.get("display_number", 0);
				auto res = screen.get_child("resolution");
				int width = res.get("width", 1920);
				int height = res.get("height", 1080);

				if (screens.contains(screenId)) {
					screens[screenId].displayNumber = displayId;
					calculateScreenCoordinates(screenId, width, height);
				}
			}
		}
		catch (const boost::property_tree::ptree_bad_path& e) {
			BOOST_LOG_TRIVIAL(error) << "[pupplayer] couldn't read screen definitions";
		}
	}

	// remove inactive screens
	for (auto itr = screens.cbegin(); itr != screens.cend(); ) {
		if (itr->second.hasDisplay()) {
			++itr;
		} else {
			itr = screens.erase(itr);
		}
	}

	// set default videos
	for (auto& pair : screens) {
		auto& screen = pair.second;
		if (screen.playFile != "") {
			screen.currentFile = screen.playList+"/"+screen.playFile;
			screen.loopCurrentFile = true;
		}
	}

	BOOST_LOG_TRIVIAL(info) << "[pupplayer] read " << this->triggers.size() << " triggers, "
		<< this->screens.size() << " screens and "
		<< this->playlists.size() << " playlists";

	initializeScreens();

	return true;
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

string PUPPlayer::name() const
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
			vector<string>parts;
			boost::split(parts, event, boost::is_any_of(":"));
			if (parts.size() == 4) {
				string filename = parts[1];
				int screenNumber = parseInteger(parts[2], -1);
				bool loop = parseBool(parts[3], false);
				if (screens.contains(screenNumber)) {
					startVideoPlayback(filename, screens[screenNumber], true);
				}
			}

		}
		else if (event.starts_with("defaultvideo:")) {
			vector<string>parts;
			boost::split(parts, event, boost::is_any_of(":"));
			if (parts.size() == 2) {
				int screenNumber = parseInteger(parts[1], -1);
				if (screenNumber >= 0) {
					playDefaultVideo(screenNumber);
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

	if (! screens.contains(triggerData.screennum)) {
		BOOST_LOG_TRIVIAL(info) << "[pupplayer] trigger " << trigger << " screen " << triggerData.screennum << " not configured, ignoring";
		return;
	}
	auto& triggerScreen = screens[triggerData.screennum];


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
		(triggerData.priority == matchingPlayerState.priority) && (triggerData.action == TriggerAction::SKIP_SAME_PRIORITY))
	{
		BOOST_LOG_TRIVIAL(debug) << "[pupplayer] ignoring trigger " << trigger << " with priority " << triggerData.priority
			<< " as a player with priority " << matchingPlayerState.priority << " is still running on screen " << triggerData.screennum;
		return;
	}

	// When stopping make sure that the player is really stopped before going on with the next events
	if (triggerData.action == TriggerAction::STOP_FILE) {
		stopVideoPlayback(triggerScreen, true);
		BOOST_LOG_TRIVIAL(debug) << "[pupplayer] trigger " << trigger << " stopped playback on screen  " << triggerData.screennum;
		return;
	}

	if (triggerData.action == TriggerAction::SET_BACKGROUND) {

	}

	// TODO: Handle more trigger loop types
	//  SPLASH_RESUME,SET_BACKGROUND

	bool loop = false;
	// TODO: What's the difference between "loop" and "loopFile"?
	if ((triggerData.action == TriggerAction::LOOP) || (triggerData.action == TriggerAction::LOOP_FILE)) {
		loop = true;

	}

	if (playfile == "") {
		BOOST_LOG_TRIVIAL(debug) << "[pupplayer] trigger " << trigger << " no playfile, doing nothing ";
		return;
	}

	playerStates[triggerData.screennum].priority = triggerData.priority;

	BOOST_LOG_TRIVIAL(error) << "[pupplayer] trigger " << trigger << " starting " << playfile;
	startVideoPlayback(playfile, triggerScreen, loop);

}

bool PUPPlayer::updatePlayerState()
{
	BOOST_LOG_TRIVIAL(error) << "[pupplayer] updatePlayerState not implemented";
	return false;
}

PUPPlayer::PUPPlayer(int screenNumber)
{
}

const vector<string> PUPPlayer::getFilesForPlaylist(string playlist) const
{
	vector<string> res;

	for (const auto f : filesystem::directory_iterator(basedir + "/" + playlist)) {
		string file = f.path();
		if (hasSupportedExtension(file)) {
			res.push_back(file);
		}
	}
	return res;
}

const vector<string> PUPPlayer::getFilesForScreen(int screenId) const {
	vector<string> res;

	// get playfiles from triggers
	for (const auto t : triggers) {
		if (t.second.screennum == screenId) {
			if (t.second.playfile != "") {
				res.push_back(basedir + "/" + t.second.playlist + "/" + t.second.playfile);
			}
			const vector playlistFiles = getFilesForPlaylist(t.second.playlist);
			res.insert(std::end(res), std::begin(playlistFiles), std::end(playlistFiles));
		}
	}

	// get playfile from screen
	auto screen = screens.at(screenId);
	if (screen.playFile != "") {
		res.push_back(basedir + "/" + screen.playList + "/" + screen.playFile);
	}

	return res;
}

