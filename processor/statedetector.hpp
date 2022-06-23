#pragma once

#include <map>
#include <vector>

#include "patternmatcher.hpp"
#include "frameprocessor.hpp"
#include "../util/glob.hpp"
#include "../util/data.hpp"

class MatchParameters {

public:
	// coordinates
	int y = 0;
	vector<int> x;

	string matcherName;

	string action;

	bool isValid();
};

class MatcherRule {

public:
	vector<MatchParameters> matchParameters;

	bool isValid();
};


class StateDetector : public DMDFrameProcessor {

public:
	string getMatch(const DMDFrame& frame, const MatchParameters& mp, vector<Rectangle>& matchRectangles);
	virtual DMDFrame processFrame(DMDFrame& f) override;
	void setVariable(string name, string value);
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

private:

	DMDPalette palette;
	bool enableColorisation = false;

	bool loadRules(string filename);

	map<string, PatternMatcher> matchers;
	vector<MatcherRule> rules;
};