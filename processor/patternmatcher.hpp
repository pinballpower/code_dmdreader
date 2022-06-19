#include <string>
#include <map>

#include "../dmd/dmdframe.hpp"

using namespace std;

class PatternMatcher {

public:
	PatternMatcher();
	PatternMatcher(string patternFile);

	std::optional<char> matchAt(const DMDFrame& frame, int x, int y) const;

	bool hasPatterns() const;

	int width = 0;
	int height = 0;

	string name = "";

private:

	map<char, DMDFrame> patterns;

};