#include <cassert>

#include <boost/log/trivial.hpp>

#include "streamhelper.hpp"
#include "../../util/bithelper.hpp"
#include "vnianimationframe.hpp"
#include "vnianimationplane.hpp"


VniAnimationFrame::VniAnimationFrame(ifstream& is, int file_version)
{
	int plane_size = read_int16_be(is);
	BOOST_LOG_TRIVIAL(trace) << "[vinanimationframe] offset " << is.tellg() << " plane_size is " << plane_size;
	delay = read_u16_be(is);

	if (file_version >= 4) {
		hash = read_u32_be(is);
	}

	bit_length = read_u8(is);
	BOOST_LOG_TRIVIAL(trace) << "[vinanimationframe] offset " << is.tellg() << " bit_length is " << bit_length;
	assert(bit_length <= 8);

	if (file_version < 3) {
		readPlanes(is, plane_size);
	}
	else {
		bool compressed = (read_u8(is) != 0);
		if (!compressed) {
			readPlanes(is, plane_size);

		}
		else {
			BOOST_LOG_TRIVIAL(error) << "[vinanimation] compressed planed not yet supported";
			// TODO: implement compressed planes
		}
	}
}

void VniAnimationFrame::readPlanes(ifstream& is, int plane_size)
{
	for (int i = 0; i < bit_length; i++) {
		uint8_t marker = read_u8(is);
		if (marker == 0x6d) {
			BOOST_LOG_TRIVIAL(trace) << "[vinanimation] offset " << is.tellg() << " read mask";
			mask = vector<uint8_t>(plane_size);
			is.read((char*)&mask[0], plane_size);
			reverse_byte_vector(mask);
			bool is_full = true;
			for (int i = 0; i < plane_size; i++) {
				if (mask[i] != 0xff) is_full = false;
			}
			if (is_full) {
				BOOST_LOG_TRIVIAL(trace) << "[vinanimation] no mask ";
			}
		}
		else {
			BOOST_LOG_TRIVIAL(trace) << "[vinanimation] offset " << is.tellg() << " read plane ";
			VniAnimationPlane plane = VniAnimationPlane(is, plane_size, marker);
			planes.push_back(std::move(plane));
		}
	}

	combine_planes(plane_size);
}


