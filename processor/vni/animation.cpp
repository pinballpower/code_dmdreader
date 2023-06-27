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

const vector<AnimationFrame> Animation::get_frames() const
{
	return frames;
}

const AnimationFrame Animation::get_frame(int index) const
{
	return frames[index];
}

Animation::Animation()
{
	width = 0;
	height = 0;
	//animation_duration = 0;
	offset = 0;
	switch_mode = ModePalette;
}
