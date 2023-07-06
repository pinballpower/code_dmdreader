#include "patterndetector.hpp"

#include <filesystem>

#include "palettecolorizer.hpp"
#include "../util/data.hpp"
#include "../util/glob.hpp"

using namespace std;


void logMatches(int frameId, string matcherName, int y, string xPositions, bool writeHeader, ofstream& of) {
    if (writeHeader) {
        of << "\nFrame: " << frameId << "\n";
    }
    of << "{ \n" 
        << "  \"matcher\": \"" << matcherName << "\", \n"
        << "  \"y\": " << y << ", \n"
        << "  \"x\": [" << xPositions << "],\n"
        << "  \"action\": \"\"\n"
        << "},\n";
}



DMDFrame PatternDetector::processFrame(DMDFrame& f)
{
    bool writeHeader = true;
    vector<Rectangle> matchRectangles;
    for (const auto matcher : matchers) {
        for (int y = 0; y <= f.getHeight() - matcher.height; y++) {
            string matches = "";
            string positions = "";
            for (int x = 0; x <= f.getWidth() - matcher.width; x++) {
                auto match = matcher.matchAt(f, x, y);
                if (match.has_value()) {
                    matches += match.value();
                    positions += to_string(x) + ",";
                    matchRectangles.push_back(Rectangle(x, y, matcher.width, matcher.height));
                }
            }
            if (matches.length() > 0) {
                string patternPositions = matcher.name+":"+positions.substr(0, positions.length() - 1) + "x" + to_string(y);
                if (detectedPatterns.contains(patternPositions)) {
                    detectedPatterns[patternPositions] += 1;
                }
                else {
                    detectedPatterns[patternPositions] = 1;
                }
                BOOST_LOG_TRIVIAL(info) << "[PatternDetector] frame " << f.getId() << ": " << patternPositions << ": " << matches;
                logMatches(f.getId(), matcher.name, y, positions.substr(0, positions.length() - 1), writeHeader, jsonOutput);
                writeHeader = false; // write it only once per frame
            }
        }
    }

    if (enableColorisation) {
        return highlightRectangles(f, palette, matchRectangles);
    }
    else {
        return f;
    }
}


bool PatternDetector::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	string patternFiles = pt_source.get("patterns", "*.png");
    enableColorisation = pt_source.get("color_frames", false);
    string jsonOutputName = pt_source.get("json_output", "detectedpatterns.json");
   
    jsonOutput.open(jsonOutputName);

    for (const auto& path : glob::rglob(patternFiles))
    {
        const auto basename = path.filename().string();
        auto matcher = PatternMatcher(path.string());
        if (matcher.hasPatterns()) {
            matchers.push_back(matcher);
        }
    }

    if (matchers.size() == 0) {
        BOOST_LOG_TRIVIAL(warning) << "[PatternDetector] couldn't load any match patterns from " << patternFiles;
        return false;
    }

    BOOST_LOG_TRIVIAL(info) << "[PatternDetector] loaded " << matchers.size() << " pattern(s) from " << patternFiles;

    if (enableColorisation) {
        palette = DMDPalette::pd_4_ffc300();
    }

    return true;
}

void PatternDetector::close()
{
    jsonOutput.close();

    BOOST_LOG_TRIVIAL(info) << "[PatternDetector] detected patterns: ";
    for (auto pattern : detectedPatterns) {
        BOOST_LOG_TRIVIAL(info) << "[PatternDetector] " << pattern.first << ": " << pattern.second;
    }
}


PatternDetector::PatternDetector() {
    name = "PatternDetector";
}
