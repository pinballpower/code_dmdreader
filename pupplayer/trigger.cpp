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
        BOOST_LOG_TRIVIAL(error) << "[puptrigger] can't parse trigger line, need at least 14 fields: \"" << configLine << "\"";
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

        loop = TriggerLoop::DEFAULT;
        if (fields[12] == "SkipSamePrty") {
            loop = TriggerLoop::SKIP_SAME_PRIORITY;
        }
        else if (fields[12] == "SplashResume") {
            loop = TriggerLoop::SPLASH_RESUME;
        }
        else if (fields[12] == "Loop") {
            loop = TriggerLoop::LOOP;
        }
        else if (fields[12] == "LoopFile") {
            loop = TriggerLoop::LOOP_FILE;
        }
        else if (fields[12] == "SkipSamePrty") {
            loop = TriggerLoop::SKIP_SAME_PRIORITY;
        }
        else if (fields[12] == "StopFile") {
            loop = TriggerLoop::STOP_FILE;
        }
        else if (fields[12] == "SetBG") {
            loop = TriggerLoop::SET_BACKGROUND;
        }
        else {
            BOOST_LOG_TRIVIAL(error) << "[puptrigger] don't know trigger loop type " << fields[12] << ", using default";
        }
        defaults = parseInteger(fields[13],0);
    }
    catch (...) {
        BOOST_LOG_TRIVIAL(error) << "[puptrigger] can't parse trigger line  \"" << configLine << "\"";
    }

}

bool PUPTrigger::isValid()
{
    return id >= 0;
}
