#include "pupplayer.hpp"

#include <queue>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "../drm/drmhelper.hpp"
#include "../drm/videofile.hpp"
#include "trigger.hpp"

int flex_stoi(const string &s, int defaultValue) {
    if (s == "") {
        return defaultValue;
    }
    return std::stoi(s);
}

enum class CSVState {
    UnquotedField,
    QuotedField,
    QuotedQuote
};

vector<string> splitLine(const string &line) {
    CSVState state = CSVState::UnquotedField;
    std::vector<std::string> fields{ "" };
    size_t i = 0; // index of the current field
    for (char c : line) {
        switch (state) {
        case CSVState::UnquotedField:
            switch (c) {
            case ',': // end of field
                fields.push_back(""); i++;
                break;
            case '"': state = CSVState::QuotedField;
                break;
            default:  fields[i].push_back(c);
                break;
            }
            break;
        case CSVState::QuotedField:
            switch (c) {
            case '"': state = CSVState::QuotedQuote;
                break;
            default:  fields[i].push_back(c);
                break;
            }
            break;
        case CSVState::QuotedQuote:
            switch (c) {
            case ',': // , after closing quote
                fields.push_back(""); i++;
                state = CSVState::UnquotedField;
                break;
            case '"': // "" -> "
                fields[i].push_back('"');
                state = CSVState::QuotedField;
                break;
            default:  // end of quote
                state = CSVState::UnquotedField;
                break;
            }
            break;
        }
    }
    return fields;
}

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
    triggers = readConfigFile<PUPTrigger>("/home/matuschd/code_dmdreader/samples/spagb_100b2s/triggers.pup");
    screens = readConfigFile<PUPScreen>("/home/matuschd/code_dmdreader/samples/spagb_100b2s/screens.pup");
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

