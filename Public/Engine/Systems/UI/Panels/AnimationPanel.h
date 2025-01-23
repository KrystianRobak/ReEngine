//
// Created by ragberr on 21.01.2025.
//
#pragma once

#include <glm/vec2.hpp>

#include "UIComponent.h"

class AnimationPanel  : public UIComponent {

public:

    virtual void Render();

    float value = 0;
    bool IsRunning = false;
};
