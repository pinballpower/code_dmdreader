#include <boost/log/trivial.hpp>

#include "pupplayer.hpp"
#include "screen.hpp"

PUPScreen::PUPScreen(string configLine)
{
    std::vector<std::string> fields = splitLine(configLine);

    if (fields.size() < 8) {
        BOOST_LOG_TRIVIAL(error) << "can't parse screen line, need at least 8 fields: \"" << configLine << "\"";
    }

    try {
        screenNum = flex_stoi(fields[0]);
        screenDescription = fields[1];
        playList = fields[2];
        playFile = fields[3];
        loopit = fields[4];

        active = true;

        priority = flex_stoi(fields[6]);
        customPos= fields[7];
    }
    catch (...) {
        BOOST_LOG_TRIVIAL(error) << "can't parse screen line  \"" << configLine << "\"";

    }
}

bool PUPScreen::isValid()
{
    return screenNum >= 0;
}
