#include <cstdint>
#include <fstream>

#include <boost/log/trivial.hpp>

#include "vnianimationplane.hpp"
#include "../../util/bithelper.hpp"
#include "../../util/crc32.hpp"

VniAnimationPlane::VniAnimationPlane(std::ifstream& is, int plane_size, uint8_t marker)
{
	this->marker = marker;
	data = vector<uint8_t>(plane_size);
	is.read((char*) &data[0], plane_size);
	uint32_t chk = crc32vect(data,true);
	BOOST_LOG_TRIVIAL(trace) << "[vinanimationplane] offset " << is.tellg() << " read animation plane " << marker+0 << ", length " << plane_size << std::hex << " checksum " << chk;
};
