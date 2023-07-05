#pragma once

#include <cstdint>
#include <map>

#include "animation.hpp"

class AnimationSet
{
public:
	int version;
	Animation& find(int offset);

	const map<uint32_t, Animation> getAnimations() const;

protected:
	map<uint32_t, Animation> animations;

};