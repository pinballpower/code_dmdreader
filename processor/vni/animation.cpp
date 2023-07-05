#include "animation.hpp"

int Animation::num_frames() const
{
	return frames.size();
}

int Animation::bit_length() const
{
	if (frames.size() > 0) {
		return frames[0].bit_length;
	}
	else {
		return 0;
	}
}

int Animation::size() const
{
	return frames.size();
}

const vector<AnimationFrame> Animation::getFrames() const
{
	return frames;
}

const AnimationFrame Animation::getFrame(int index) const
{
	return frames[index];
}

const std::optional<AnimationFrame> Animation::getNextFrame() {
	if (!(isActive())) {
		return {};
	}
	
	auto res = getFrame(current_frame);
	current_frame += 1;

	if (current_frame >= frames.size()) {
		stop();
	}

	return res;
}


Animation::Animation()
{
	width = 0;
	height = 0;
	//animation_duration = 0;
	offset = 0;
	switch_mode = ModePalette;
}


void Animation::start() {
	current_frame = 0;
}

void Animation::stop() {
	current_frame = -1;
}

bool Animation::isActive() const {
	return (current_frame >= 0) && 
		(current_frame < frames.size());
}