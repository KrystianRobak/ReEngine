#pragma once
#include "UIComponent.h"
#include "Types.h"




class ControlPanel : public UIComponent
{
public:

    ControlPanel()
    {
    }

    void Render();
private:
    int type;
};

