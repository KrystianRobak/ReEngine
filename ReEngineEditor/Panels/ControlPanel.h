#pragma once
#include "UIComponent.h"
#include "Types.h"
#include "Engine/Core/Coordinator/Coordinator.h"




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

