#pragma once

#include <string>
#include <vector>

#include "switchmode.hpp"
#include "animationframe.hpp"

using namespace std;

enum AnimationEditMode
{
	Replace, Mask, Fixed
};

class Animation {
public:
	Animation();

	string name;
	// unsigned int animation_duration;

	/// <summary>
	/// defines how the animation is applied
	/// </summary>
	SwitchMode switch_mode;

	int width;
	int height;


	int num_frames() const;
	int bit_length() const;
	int size() const;

	/// <summary>
	/// Technically this is the offset in the animations file, but it is also used as a unique ID of the animation
	/// </summary>
	uint32_t offset;

	const vector<AnimationFrame> get_frames() const;
	const AnimationFrame get_frame(int index) const;

protected:
	/// <summary>
	/// the frames of the animation
	/// </summary>
	vector<AnimationFrame> frames;

	vector<vector<uint8_t>> masks;

};
