#include <boost/log/trivial.hpp>

#include "vnianimation.hpp"
#include "vnianimationframe.hpp"
#include "streamhelper.hpp"
#include "../../util/bithelper.hpp"
#include "../../dmd/color.hpp"

VniAnimation::VniAnimation(ifstream& is, int file_version) {

	// offset
	offset = is.tellg();

	// animations name
	int name_length = read_int16_be(is);
	name = name_length > 0 ? read_string(is, name_length) : "<undefined>";
	BOOST_LOG_TRIVIAL(trace) << "[vinanimation] offset " << is.tellg() << " read animation name as " << name;

	// skip 16 bytes of unused data
	is.ignore(16);

	int num_frames = read_int16_be(is);
	if (num_frames < 0) {
		num_frames += 65536;
	}
	BOOST_LOG_TRIVIAL(trace) << "[vinanimation] offset " << is.tellg() << " read animation frame count as " << num_frames;

	if (file_version >= 2) {
		readPalettesAndColors(is);
	}
	if (file_version >= 3) {
		read_u8(is); // AnimationEditMode not used
	}
	if (file_version >= 4) {
		width = read_int16_be(is);
		height = read_int16_be(is);
		BOOST_LOG_TRIVIAL(trace) << "[vinanimation] offset " << is.tellg() << " read dimension as " << width << "x" << height;
	}
	if (file_version >= 5)
	{
		int num_masks = read_int16_be(is);
		BOOST_LOG_TRIVIAL(trace) << "[vinanimation] offset " << is.tellg() << " read mask count as " << num_masks;
		for (int i = 0; i < num_masks; i++)
		{
			int locked = read_u8(is);
			int getSize = read_int16_be(is);
			vector<uint8_t> mask = vector<uint8_t>(getSize);
			is.read((char*)&mask[0], getSize);
			reverse_byte_vector(mask);
			masks.push_back(mask);
		}
	}

	if (file_version >= 6)
	{
		int linked_animation = read_u8(is);
		int getSize = read_int16_be(is);
		string anim_name = getSize > 0 ? read_string(is, getSize) : "<undefined>";
		uint32_t startFrame = read_u32_be(is);
		BOOST_LOG_TRIVIAL(trace) << "[vinanimation] offset " << is.tellg() << " read anim name " << anim_name;
	}

	BOOST_LOG_TRIVIAL(trace) << "[vinanimation] offset " << is.tellg() << " reading " << num_frames << " frame(s) for animation " << name;

	for (int i = 0; i < num_frames; i++) {
		VniAnimationFrame frame = VniAnimationFrame(is, file_version);

		frames.push_back(frame);
	}
};

void VniAnimation::readPalettesAndColors(ifstream& is)

{
	read_int16_be(is); // palette index unused
	int num_colors = read_int16_be(is);
	if (num_colors <= 0) {
		return;
	}

	BOOST_LOG_TRIVIAL(trace) << "[vinanimation] offset " << is.tellg() << " skipping " << num_colors*3 << " byte of embedded palette data ";
	is.ignore(num_colors * 3); // embedded colors unused
};



