#include "animation.hpp"

#include "../../util/time.hpp"

int Animation::getNumFrames() const
{
	return frames.size();
}

int Animation::getBitLength() const
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

	auto now = getMicrosecondsTimestamp();
	auto timeCurrentFrame = frames[current_frame].delay*1000;
	
	if ((now - frameStartMicrosecs) > timeCurrentFrame) {
		// time for current frame has expired, skip to next frame
		current_frame += 1;
		frameStartMicrosecs += timeCurrentFrame;
	}

	if (current_frame >= frames.size()) {
		stop();
		return frames[frames.size()-1];
	}

	return frames[current_frame];
}


Animation::Animation()
{
	width = 0;
	height = 0;
	//animation_duration = 0;
	offset = 0;
	switch_mode = ModePalette;
}


void Animation::start(bool restart) {
	if ((restart) || (!isActive())) {
		current_frame = 0;
		frameStartMicrosecs = getMicrosecondsTimestamp();
	}
}

void Animation::start() {
	start(false);
}

void Animation::stop() {
	current_frame = frames.size();
}

bool Animation::isActive() const {
	return (current_frame >= 0) && 
		(current_frame < frames.size());
}

const int Animation::framesLeft() const {
	return frames.size() - current_frame;
}