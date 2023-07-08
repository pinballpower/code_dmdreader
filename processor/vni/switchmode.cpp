#include "switchmode.hpp"

bool usesAnimationFrame(SwitchMode mode) {
	return (
		(mode == ModeColorMask) ||
		(mode == ModeColorMask) ||
		(mode == ModeLayeredColorMask) ||
		(mode == ModeFollowReplace) ||
		(mode == ModeMaskedReplace)
		);
}
