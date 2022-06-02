#include "internalpupplayer.hpp"
#include "pupplayer.hpp"

#include <queue>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include "../drm/drmframebuffer.hpp"
#include "../drm/drmhelper.hpp"
#include "../drm/videofile.hpp"
#include "../util/image.hpp"
#include "trigger.hpp"


InternalPUPPlayer::~InternalPUPPlayer()
{
	stop();
}

void InternalPUPPlayer::addFinishNotify(int screenId) {
	videoPlayers[screenId]->setNotify(this);
}

bool InternalPUPPlayer::initVideoScreen(int screenId, int displayNumber, int& planeIndex) {

	CompositionGeometry fullscreen = DRMHelper::getFullscreenResolution(displayNumber);
	if (fullscreen.isUndefined()) {
		return false;
	}

	auto screen = screens[screenId];

	CompositionGeometry composition;
	composition.x = screen.x * fullscreen.width;
	composition.y = screen.y * fullscreen.height;
	composition.width = screen.width * fullscreen.width;
	composition.height = screen.height * fullscreen.height;

	videoPlayers[screenId] = std::make_unique<VideoPlayer>(displayNumber, planeIndex, composition, screenId);
	addFinishNotify(screenId);
	playerStates[screenId] = PlayerState();
	planeIndex++;

	return true;
}

bool InternalPUPPlayer::initFrameBufferScreen(int screenId, int displayNumber, int& planeIndex) {

	CompositionGeometry fullscreen = DRMHelper::getFullscreenResolution(displayNumber);
	if (fullscreen.isUndefined()) {
		return false;
	}

	auto screen = screens[screenId];

	CompositionGeometry composition;
	composition.x = screen.x * fullscreen.width;
	composition.y = screen.y * fullscreen.height;
	composition.width = screen.width * fullscreen.width;
	composition.height = screen.height * fullscreen.height;

	//frameBuffers[screenId] = std::make_unique<DRMFrameBuffer>(displayNumber, planeIndex, composition);
	//planeIndex++;

	return true;
}

void InternalPUPPlayer::playerHasFinished(int screenId) {
	BOOST_LOG_TRIVIAL(info) << "[pupplayer] playback on screen " << screenId << " has been finished";
}

void InternalPUPPlayer::playbackFinished(int playerId, VideoPlayerFinishCode finishCode)
{
	BOOST_LOG_TRIVIAL(warning) << "[pupplayer] video player " << playerId << " finished playback";
	playerStates[playerId].playing = false;
	if (finishCode != VideoPlayerFinishCode::STOPPED_FOR_NEXT_VIDEO) {
		sendEvent("defaultvideo:" + std::to_string(playerId));
	}
}



bool InternalPUPPlayer::updatePlayerState() {
	for (auto& playerState : playerStates) {
		int playerId = playerState.first;
		auto& player = videoPlayers[playerId];
		if (!player) {
			BOOST_LOG_TRIVIAL(warning) << "[pupplayer] got a null player, something is terribly wrong :( ";
			continue;
		}
		if (!player->isPlaying()) {
			playerState.second.playing = false;
			playerState.second.priority = -1;
		}
	}
	return true;
}

bool InternalPUPPlayer::startVideoPlayback(string filename, PUPScreen& screen, bool loop) {
	// TODO: Preload video files 

	auto& player = videoPlayers[screen.screenNum];
	if (!player) {
		BOOST_LOG_TRIVIAL(warning) << "[pupplayer] got a null player, something is terribly wrong :( ";
		return false;
	}

	if (PUPPlayer::hasSupportedExtension(filename)) {
		if (usePreloader) {
			player->startPlayback(videoFileLoader.getFile(filename), loop);
		}
		else {
			player->startPlayback(std::make_unique<VideoFile>(filename));
		}
		playerStates[screen.screenNum].playing = true;
		BOOST_LOG_TRIVIAL(info) << "[pupplayer] play file " << filename << " on screen " << screen.screenNum;
	}
	else {
		BOOST_LOG_TRIVIAL(debug) << "[pupplayer] ignoring play file " << filename << ", unsupported extension";
		return false;
	}

	return true;
}

bool InternalPUPPlayer::stopVideoPlayback(PUPScreen& screen, bool waitUntilStopped)
{
	return false;
}

InternalPUPPlayer::InternalPUPPlayer(int screenNumber)
{
}


bool InternalPUPPlayer::initializeScreens() {
	// TODO: initialize video screens
	return false;
}

