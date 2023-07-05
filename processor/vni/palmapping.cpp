#include <boost/log/trivial.hpp>

#include "palmapping.hpp"
#include "switchmode.hpp"
#include "streamhelper.hpp"

bool PaletteMapping::IsAnimation() const
{
	return ((mode != ModeEvent) && (mode != ModePalette));
}

PaletteMapping::PaletteMapping()
{
	checksum = 0;
	duration = 0;
	mode = SwitchMode::ModeUndefined;
	offset = 0;
	palette_index = 0;
}

PaletteMapping::PaletteMapping(istream& is)
{
	checksum = read_u32_be(is);
	BOOST_LOG_TRIVIAL(trace) << "[palettemapping] offset " << is.tellg() << " read checksum as " << std::hex << checksum;
	mode = (SwitchMode)read_u8(is);
	BOOST_LOG_TRIVIAL(trace) << "[palettemapping] offset " << is.tellg() << " read mode as " << mode;
	palette_index = read_u16_be(is);
	BOOST_LOG_TRIVIAL(trace) << "[palettemapping] offset " << is.tellg() << " read index as " << palette_index;
	if (mode == ModePalette) {
		duration = read_u32_be(is);
		BOOST_LOG_TRIVIAL(trace) << "[palettemapping] offset " << is.tellg() << " read duration as " << duration;
		offset = 0;
	} else {
		offset = read_u32_be(is);
		BOOST_LOG_TRIVIAL(trace) << "[palettemapping] offset " << is.tellg() << " read offset as " << offset;
		duration = 0;
	}
}


void PaletteMapping::start() {
	framesLeft = duration;
}

void PaletteMapping::stop() {
	framesLeft = 0;

}

bool PaletteMapping::nextFrame() {
	if (framesLeft > 0) {
		framesLeft--;
	}
	return framesLeft != 0;
}

bool PaletteMapping::isActive() {
	return framesLeft != 0;
}

