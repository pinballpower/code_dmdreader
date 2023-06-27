#pragma once

#include <string>

#include "animationset.hpp"

using namespace std;

class VniAnimationSet : public AnimationSet
{
public:
	VniAnimationSet();
	VniAnimationSet(string filename);

	int max_height=0;
	int max_width = 0;
};
