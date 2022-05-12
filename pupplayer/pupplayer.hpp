#pragma once

#include <vector>

#include "../drm/videoplayer.hpp"
#include "../services/service.hpp"


class PUPPlayer : public Service {

public:
	PUPPlayer(int screenNumber = 0);
	~PUPPlayer();

	void playEvent(int event);

private:
	vector <std::shared_ptr<VideoPlayer>> players;
};