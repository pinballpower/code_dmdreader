#include "statedetector.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>


MatchParameters readParameters(const boost::property_tree::ptree& ptree) {
    MatchParameters result;
    result.matcherName = ptree.get("matcher", "");
    result.y = ptree.get("y", -1);
    BOOST_FOREACH(const boost::property_tree::ptree::value_type & x, ptree.get_child("x")) {
        auto myX = x.second.get_value(-1);
        if (myX >= 0) {
            result.x.push_back(myX);
        }
    }
    result.action = ptree.get("action", "");

    return result;
}


bool MatchParameters::isValid()
{
    return (matcherName != "") && x.size() > 0 && y>=0 && (action != "");
}



DMDFrame StateDetector::processFrame(DMDFrame& f)
{
    return DMDFrame();
}

bool StateDetector::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
    string patternFiles = pt_source.get("patterns", "*.png");
    enableColorisation = pt_source.get("color_frames", false);
    string rulesFile = pt_source.get("rules", "");

    if (rulesFile == "") {
        BOOST_LOG_TRIVIAL(error) << "[StateDetector] no rules file defined, aborting";
        return false;
    }

    int countMatchers = 0;
    for (const auto& path : glob::rglob(patternFiles))
    {
        const auto basename = path.filename().string();
        auto matcher = PatternMatcher(path.string());
        if (matcher.hasPatterns()) {
            matchers[matcher.name] = matcher;
            countMatchers++;
        }
    }

    if (countMatchers > 0) {
        BOOST_LOG_TRIVIAL(info) << "[StateDetector] loaded " << countMatchers << " pattern files";
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "[StateDetector] no pattern files detected, aborting";
        return false;
    }

    if (!loadRules(rulesFile)) {
        return false;
    }

    return true;
}

bool StateDetector::loadRules(string filename)
{
    ifstream rulesFile(filename);
    if (!rulesFile) {
        BOOST_LOG_TRIVIAL(fatal) << "[StateDetector] can't read JSON configuration file " << filename << ", aborting";
        return false;
    }

    boost::property_tree::ptree pt;
    try {
        boost::property_tree::json_parser::read_json(filename, pt);
        BOOST_LOG_TRIVIAL(info) << "[StateDetector] using rules file " << filename;
    }
    catch (const boost::property_tree::json_parser::json_parser_error e) {
        BOOST_LOG_TRIVIAL(fatal) << "[StateDetector] couldn't parse JSON rules file " << filename << ": " << e.what();
        return false;
    }

    try {
        BOOST_FOREACH(const boost::property_tree::ptree::value_type & rule, pt.get_child("rules")) {
            MatcherRule matcherRule;
            BOOST_FOREACH(const boost::property_tree::ptree::value_type & rulePart, rule.second) {
                auto mp  = readParameters(rulePart.second);
                if (mp.isValid()) {
                    matcherRule.matchParameters.push_back(mp);

                }
            }
            if (matcherRule.isValid()) {
                this->rules.push_back(matcherRule);
            }
        }
    }
    catch (const boost::property_tree::ptree_bad_path& e) {}

    return true;
}




bool MatcherRule::isValid()
{
    return matchParameters.size() > 0;
}
