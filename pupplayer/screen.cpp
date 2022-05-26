#include <boost/log/trivial.hpp>

#include "pupplayer.hpp"
#include "screen.hpp"

PUPScreen::PUPScreen()
{
    screenNum = -1;
}

PUPScreen::PUPScreen(string configLine)
{
    std::vector<std::string> fields = splitLine(configLine);

    if (fields.size() < 8) {
        BOOST_LOG_TRIVIAL(error) << "[pupscreen] can't parse screen line, need at least 8 fields: \"" << configLine << "\"";
    }

    try {
        screenNum = parseInteger(fields[0]);
        screenDescription = fields[1];
        playList = fields[2];
        playFile = fields[3];
        loopit = fields[4];

        active = true;

        priority = parseInteger(fields[6]);

        string customPos= fields[7];
        if (customPos != "") {
            vector<string> posFields = splitLine(customPos);
            if (posFields.size() < 5) {
                BOOST_LOG_TRIVIAL(error) << "[pupscreen] can't parse screen position, need at least 5 fields: \"" << customPos << "\"";
            }
            else {
                parentScreen = parseInteger(posFields[0]);
                // convert percentage to 0-1 range
                x = parseFloat(posFields[1])/100;
                y = parseFloat(posFields[2])/100;
                width = parseFloat(posFields[3])/100;
                height = parseFloat(posFields[4])/100;
            }
        }

    }
    catch (...) {
        BOOST_LOG_TRIVIAL(error) << "[pupscreen] can't parse screen line  \"" << configLine << "\"";

    }
}

bool PUPScreen::isValid() const
{
    return screenNum >= 0;
}


bool PUPScreen::hasDisplay() const
{
    return displayNumber >= 0;
}
