#pragma once

#include <string>
#include <vector>

#include <boost/log/trivial.hpp>

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


	int getNumFrames() const;
	int getBitLength() const;
	int size() const;

	/// <summary>
	/// Technically this is the offset in the animations file, but it is also used as a unique ID of the animation
	/// </summary>
	uint32_t offset;

	const vector<AnimationFrame> getFrames() const;
	const AnimationFrame getFrame(int index) const;
	const std::optional<AnimationFrame> getNextFrame(bool srcFrameChanged);
	const int framesLeft() const; // how many frames are left in the current animation

	// start the animation. If restart is true, it will always start with frame 0, otherwise it won't effect an animation that's
	// already active
	void start(bool restart);
	void start(); // start with restart=False

	void stop();
	bool isActive() const;

	bool sameAnimation(Animation anim) const;

protected:
	/// <summary>
	/// the frames of the animation
	/// </summary>
	vector<AnimationFrame> frames;
	vector<vector<uint8_t>> masks;

	uint64_t frameStartMicrosecs = 0;


private:
	int current_frame = -1;

};
