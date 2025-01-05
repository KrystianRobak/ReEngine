#pragma once
#include "UIComponent.h"

#include "../Public/Engine/Core/Coordinator/Coordinator.h"

enum MenuType
{
    BaseMenu,
    AnimationMenu
};


class ControlPanel
{
public:

    ControlPanel()
    {
    }

    void Render();
private:
    int type;
};

