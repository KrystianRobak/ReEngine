//
// Created by ragberr on 22.01.2025.
//
#pragma once

#include <vector>
#include "../Core/AnimationTypes.h"

struct Animated
{
	AnimationPath AnimPath;
	Animation CurrentlyPlaying;
	bool IsPlaying;
};
