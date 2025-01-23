#pragma once
#include "UIComponent.h"
#include "Public/Engine/Core/Types.h"
#include "../Public/Engine/Core/Coordinator/Coordinator.h"




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

