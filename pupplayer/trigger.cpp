#include "trigger.hpp"
#include "pupplayer.hpp"

#include <vector>
#include <boost/log/trivial.hpp>

using namespace std;


PUPTrigger::PUPTrigger(string configLine)
{
    std::vector<std::string> fields = splitLine(configLine);

    if (fields.size() < 14) {
        BOOST_LOG_TRIVIAL(error) << "can't parse trigger line, need at least 14 fields: \"" << configLine << "\"";
    }

    try {
        id = std::stoi(fields[0]);
        if (std::stoi(fields[1])) {
            active = true;
        }
        else {
            active = false;
        }

        description = fields[2];
        trigger = fields[3];
        screennum = flex_stoi(fields[4]);
        playlist = fields[5];
        playfile = fields[6];

        volume = flex_stoi(fields[7]);
        priority = flex_stoi(fields[8]);
        length = flex_stoi(fields[9]);
        counter = flex_stoi(fields[10]);
        rest_seconds = flex_stoi(fields[11]);

        loop = fields[12];

        defaults = flex_stoi(fields[13],0);
    }
    catch (...) {
        BOOST_LOG_TRIVIAL(error) << "can't parse trigger line  \"" << configLine << "\"";

    }

}

bool PUPTrigger::isValid()
{
    return id >= 0;
}
