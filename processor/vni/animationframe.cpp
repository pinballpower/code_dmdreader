#include <cassert>

#include "animationframe.hpp"
#include "../../dmd/dmdframe.hpp"


const vector<uint8_t> AnimationFrame::get_frame_data() const
{
	return combined;
}

const vector<AnimationPlane> AnimationFrame::get_planes() const
{
	return planes;
}

DMDFrame AnimationFrame::as_dmd_frame(int width, int height) const
{
	return DMDFrame(width, height, 8, this->get_frame_data());
}

void AnimationFrame::combine_planes(int len)
{
	len = len * 8;

	int bit = 7;
	int offset = 0;

	combined.clear();

	uint8_t bits = planes.size();
	uint8_t maxval = 0xff >> (8 - bits);

	for (; len; len--) {

		uint8_t pv = 0;
		uint8_t planebit = 0;
		for (auto &p: planes) {
			// get n'th bit and set it
			pv |= ((p.getPixelData()[offset] >> bit) & 0x01) << planebit;
			planebit++;
		}

		assert(pv <= maxval);
		
		pv |= ((mask[offset] >> bit) & 0x01) << 7;

		// next pixel
		bit--;
		if (bit < 0 ) {
			offset++;
			bit = 7;
		}

		combined.push_back(pv);
	}
}
