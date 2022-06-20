#include "patterndetector.hpp"

#include <filesystem>
#include <regex>

using namespace std;

DMDFrame PatternDetector::processFrame(DMDFrame& f)
{
    for (const auto matcher : matchers) {
        for (int y = 0; y < f.getHeight() - matcher.height; y++) {
            string matches = "";
            string positions = "";
            for (int x = 0; x < f.getWidth() - matcher.width; x++) {
                auto match = matcher.matchAt(f, x, y);
                if (match.has_value()) {
                    matches += match.value();
                    positions += to_string(x) + ",";
                }
            }
            if (matches.length() > 0) {
                positions = matcher.name+":"+positions.substr(0, positions.length() - 1) + "x" + to_string(y);
                if (detectedPatterns.contains(positions)) {
                    detectedPatterns[positions] += 1;
                }
                else {
                    detectedPatterns[positions] = 1;
                }
                BOOST_LOG_TRIVIAL(info) << "[PatternDetector] frame " << f.getId() << ": " << positions << ": " << matches;
            }
        }
    }

    return f;
}

bool PatternDetector::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	string directory = pt_source.get("directory", ".");
	string pattern = pt_source.get("pattern", "*.png");

    filesystem::path folder(directory);
    if (!filesystem::is_directory(folder))
    {
        BOOST_LOG_TRIVIAL(error) << "[PatternDetector] " << directory << " is not a directory";
        return false;
    }

    vector<std::string> file_list;
    auto match_expression = regex(pattern);

    for (const auto& file : filesystem::directory_iterator(folder))
    {
        if (file.is_regular_file())
        {
            const auto basename = file.path().filename().string();
            if (regex_match(basename, match_expression)) {
                auto matcher = PatternMatcher(file.path().string());
                if (matcher.hasPatterns()) {
                    matchers.push_back(matcher);
                }
            }
        }
    }

    if (matchers.size() == 0) {
        BOOST_LOG_TRIVIAL(warning) << "[PatternDetector] couldn't load any match patterns from " << directory << "/" << pattern;
        return false;
    }

    BOOST_LOG_TRIVIAL(info) << "[PatternDetector] loaded " << matchers.size() << " pattern(s) from " << directory << "/" << pattern;
    return true;
}

void PatternDetector::close()
{
    BOOST_LOG_TRIVIAL(info) << "[PatternDetector] detected patterns: ";
    for (auto pattern : detectedPatterns) {
        BOOST_LOG_TRIVIAL(info) << "[PatternDetector] " << pattern.first << ": " << pattern.second;
    }
}
