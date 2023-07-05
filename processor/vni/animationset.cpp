#include "animationset.hpp"

Animation& AnimationSet::find(int offset)
{
	return animations[offset];
}

const map<uint32_t, Animation> AnimationSet::getAnimations() const
{
	return animations;
}
