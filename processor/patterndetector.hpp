#pragma once

#include <map>
#include <vector>

#include "patternmatcher.hpp"
#include "frameprocessor.hpp"

class PatternDetector : public DMDFrameProcessor {

public:
	virtual DMDFrame processFrame(DMDFrame& f) override;
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;
	virtual void close() override;

private:

	vector<PatternMatcher> matchers;
	map<string,int> detectedPatterns;
};