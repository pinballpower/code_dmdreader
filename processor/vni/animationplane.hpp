#pragma once

#include <cstdint>
#include <vector>

using namespace std;

/// <summary>
/// An animation plane is a 1-bit plane 
/// </summary>
class AnimationPlane {

public:
	const vector<uint8_t>& getPixelData() const;

protected:
	/// <summary>
	/// Type of plane
	/// </summary>
	uint8_t marker = 0;

	vector<uint8_t> data;
};
