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

//
// Get the next frame from an animation as follows:
// - if the animation has not been started, return nothing
// - if the current animation frame has a duration set, wait for this duration until skipping to the next frame
// - if duration of the current animation frame is 0, only skip to next frame if "srcFrameChanged" is true (which means
//   the source frame has been changed)
// 
const std::optional<AnimationFrame> Animation::getNextFrame(bool srcFrameChanged) {
	if (!(isActive())) {
		return {};
	}

	auto now = getMicrosecondsTimestamp();
	auto timeCurrentFrame = frames[current_frame].delay*1000;

	if (timeCurrentFrame == 0) {
		if (srcFrameChanged) {
			current_frame += 1;
			frameStartMicrosecs += timeCurrentFrame;
		}
	}
	else {
		if ((now - frameStartMicrosecs) > timeCurrentFrame) {
			// time for current frame has expired, skip to next frame
			current_frame += 1;
			frameStartMicrosecs += timeCurrentFrame;
		}
		else {
			auto x = now - frameStartMicrosecs;
			BOOST_LOG_TRIVIAL(debug) << "now:" << now/1000 << ", frameStartMicrosecs: " << frameStartMicrosecs/1000 << ", (now - frameStartMicrosecs):" << x/1000 << ", timeCurrentFrame: " << timeCurrentFrame/1000;
		}
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

bool Animation::sameAnimation(Animation anim) const
{
	return this->offset == anim.offset;
}

const int Animation::framesLeft() const {
	return frames.size() - current_frame;
}