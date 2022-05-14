#include "pupplayer.hpp"

#include <queue>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/trim.hpp>

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

bool PUPPlayer::initScreen(int screenId, int displayNumber) {

	// first initialize main screen
	int planeIndex = 0;
	players[screenId] = std::make_unique<VideoPlayer>(displayNumber, planeIndex, CompositionGeometry());
	playerStates[screenId] = PlayerState();
	planeIndex++;

	// Now all subscreens
	for (auto& screen : screens) {
		if ((screen.screenNum == screenId) || (screen.parentScreen == screenId)) {
			BOOST_LOG_TRIVIAL(trace) << "[pupcapture] trying to configure screen " << screen.screenNum;

			CompositionGeometry composition;
			// TODO: handle composition

			players[screen.screenNum] = std::make_unique<VideoPlayer>(displayNumber, planeIndex, composition);
			playerStates[screen.screenNum] = PlayerState();
			planeIndex++;
		}
	}

	BOOST_LOG_TRIVIAL(info) << "[pupcapture] initialized " << planeIndex << " video planes";

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
		this->screens = screens.value();
		int backglassId = -1;
		for (const auto& s : this->screens) {
			if (s.screenDescription == "Backglass") {
				backglassId = s.screenNum;
				break;
			}
		}

		if (backglassId < 0) {
			BOOST_LOG_TRIVIAL(error) << "[pupplayer] can't find backglass, not initializing PUPPlayer";
			return false;
		}
		initScreen(backglassId, displayNumber);
	}

	{
		filename = basedir + "/triggers.pup";
		auto triggerList = readConfigFile<PUPTrigger>(filename);
		if (!triggerList) {
			BOOST_LOG_TRIVIAL(error) << "[pupplayer] can't read file " << filename;
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
		if (! player->isPlaying()) {
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

	if (! triggers.contains(trigger)) {
		BOOST_LOG_TRIVIAL(error) << "[pupplayer] trigger " << trigger << " unknown, ignoring";
		return;
	}

	const auto& triggerData = triggers[trigger];

	if (! playerStates.contains(triggerData.screennum)) {
		BOOST_LOG_TRIVIAL(trace) << "[pupplayer] trigger " << trigger << " for inactive screen, ignoring";
		return;
	}

	string playfile = triggerData.playfile;
	if (playfile == "") {
		PUPPlaylist pl = playlists[triggerData.playlist];
		playfile = pl.nextFile();
	}
	else {
		playfile = basedir+"/"+triggerData.playlist + "/" + playfile;
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
	if ((triggerData.loop == TriggerLoop::LOOP) || (triggerData.loop == TriggerLoop::LOOP_FILE)) {
		loop = true;
		// TODO: What's the difference between "loop" and "loopFile"?
	}

	if (playfile == "") {
		BOOST_LOG_TRIVIAL(debug) << "[pupplayer] trigger " << trigger << " no playfile, doing nothing ";
		return;
	}

	playerStates[triggerData.screennum].priority = triggerData.priority;
	playerStates[triggerData.screennum].playing = true;
	// TODO: Preload video files 
	auto& player = players[triggerData.screennum];
	if (!player) {
		BOOST_LOG_TRIVIAL(warning) << "[pupplayer] got a null player, something is terribly wrong :( ";
		return;
	}
	player->startPlayback(make_unique<VideoFile>(playfile, true), loop);

	BOOST_LOG_TRIVIAL(error) << "[pupplayer] trigger " << trigger << " play file " << playfile;
}

PUPPlayer::PUPPlayer(int screenNumber)
{
}

