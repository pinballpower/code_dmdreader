#include "pupplayer.hpp"

#include <queue>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>

#include "../drm/drmhelper.hpp"
#include "../drm/videofile.hpp"
#include "trigger.hpp"


template <class myType>
vector<myType> readConfigFile(string filename) {
    vector<myType> result;

    ifstream is;
    is.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        is.open(filename);
    }
    catch (std::ios_base::failure e) {
        BOOST_LOG_TRIVIAL(error) << "[trigger] can't open file " << filename << ": " << e.what();
        return result;
    }

    bool eof = false;
    string line;
    // ignore first line with the field headers
    std::getline(is, line);
    while (!eof) {
        try {
            if (!std::getline(is, line)) {
                if (std::cin.eof()) {
                    eof = true;
                }
                else {
                    throw std::ios_base::failure("couldn't read file");
                }
            }
            boost::trim_right(line);

            myType obj = myType(line);
            if (obj.isValid()) {
                result.push_back(obj);
            }
        }
        catch (std::ios_base::failure e) {
            eof = true;
        }
    }
    return result;
}

PUPPlayer::~PUPPlayer()
{
}

bool PUPPlayer::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
 	return true;
}

bool PUPPlayer::start()
{
    return true;
}

void PUPPlayer::playEvent(int event)
{
}

PUPPlayer::PUPPlayer(int screenNumber)
{
}
