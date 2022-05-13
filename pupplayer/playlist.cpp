#include "playlist.hpp"
#include "pupplayer.hpp"

#include <boost/log/trivial.hpp>

PUPPlaylist::PUPPlaylist(string configLine)
{
	std::vector<std::string> fields = splitLine(configLine);

	if (fields.size() < 7) {
		BOOST_LOG_TRIVIAL(error) << "[playlist] can't parse playlist line, need at least 8 fields: \"" << configLine << "\"";
	}

	try {
		screenNum = parseInteger(fields[0]);
		folder = fields[1];
		description = fields[2];
		alphaSort = parseBool(fields[3], true);
		restSeconds = parseInteger(fields[4]);
		volume = parseInteger(fields[5]);
		priority = parseInteger(fields[6]);
	}
	catch (...) {
		BOOST_LOG_TRIVIAL(error) << "[playlist] can't parse screen line  \"" << configLine << "\"";

	}
}


bool PUPPlaylist::isValid()
{
	return screenNum >= 0;
}
