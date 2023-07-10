#include "switchmode.hpp"

string switchModeStr(SwitchMode mode)
{
	switch (mode) {
	case ModeReplace: return "Replace";
	case ModeColorMask: return "ColorMask";
	case ModeEvent: return "Event";
	case ModeFollow: return "Follow";
	case ModeLayeredColorMask: return "LayeredColorMask";
	case ModeFollowReplace: return "FollowReplace";
	case ModeMaskedReplace: return "MaskedReplace";
	}
	return "unknown";

}

bool usesAnimationFrame(SwitchMode mode) {
	return (
		(mode == ModeColorMask) ||
		(mode == ModeColorMask) ||
		(mode == ModeLayeredColorMask) ||
		(mode == ModeFollowReplace) ||
		(mode == ModeMaskedReplace) ||
		(mode == ModeReplace)
		);
}