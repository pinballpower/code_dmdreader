#pragma once

enum SwitchMode {
	ModePalette = 0,
	ModeReplace = 1,
	ModeColorMask = 2,
	ModeEvent = 3,
	ModeFollow = 4,
	ModeLayeredColorMask = 5,
	ModeFollowReplace = 6,
	ModeMaskedReplace = 7,
	ModeUndefined = 8
};


bool usesAnimationFrame(SwitchMode mode);