#pragma once

#include <cstdint>
#include <vector>

#include "animationplane.hpp"
#include "../../dmd/dmdframe.hpp"

using namespace std;

class AnimationFrame {

public:
	/// <summary>
	/// Duration of the frame
	/// </summary>
	unsigned int delay = 0;

	/// <summary>
	/// Bit length of the frames (and hence number of planes of each frame). Either 2 or 4.
	/// </summary>
	int bit_length = 0;

	/// <summary>
	/// Get the frame in 8-bit per pixel data
	/// </summary>
	/// <returns></returns>
	const vector<uint8_t> get_frame_data() const;

	const vector<AnimationPlane> get_planes() const;

	virtual DMDFrame as_dmd_frame(int width, int height) const;

protected:

	unsigned int hash = 0;

	/// <summary>
	/// Combine planes and mask and store it in combined
	/// </summary>
	/// <param name="len">Number of pixels</param>
	void combine_planes(int len);

	vector<uint8_t> mask;

	/// <summary>
	/// Combined planes and mask. Planes are bits 0-6 (even if there are less planes), mask is bit 7
	/// </summary>
	vector<uint8_t> combined;

	/// <summary>
	/// The planes, each plane is 1 bit
	/// </summary>
	vector<AnimationPlane> planes;




};