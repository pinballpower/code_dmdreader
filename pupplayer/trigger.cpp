#include "trigger.hpp"
#include "pupplayer.hpp"

#include <vector>
#include <boost/log/trivial.hpp>

using namespace std;


PUPTrigger::PUPTrigger()
{
}

PUPTrigger::PUPTrigger(string configLine)
{
    std::vector<std::string> fields = splitLine(configLine);

    if (fields.size() < 14) {
        BOOST_LOG_TRIVIAL(error) << "can't parse trigger line, need at least 14 fields: \"" << configLine << "\"";
    }

    try {
        id = parseInteger(fields[0]);
        active = parseBool(fields[1]);
        description = fields[2];
        trigger = fields[3];
        screennum = parseInteger(fields[4]);
        playlist = fields[5];
        playfile = fields[6];
        volume = parseInteger(fields[7]);
        priority = parseInteger(fields[8]);
        length = parseInteger(fields[9]);
        counter = parseInteger(fields[10]);
        rest_seconds = parseInteger(fields[11]);
        loop = fields[12];
        defaults = parseInteger(fields[13],0);
    }
    catch (...) {
        BOOST_LOG_TRIVIAL(error) << "can't parse trigger line  \"" << configLine << "\"";

    }

}

bool PUPTrigger::isValid()
{
    return id >= 0;
}
