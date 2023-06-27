#pragma once

#include <cstdint>
#include <map>

#include "animation.hpp"

class AnimationSet
{
public:
	int version;
	const Animation find(int offset);

	const map<uint32_t, Animation> get_animations() const;

protected:
	map<uint32_t, Animation> animations;

};