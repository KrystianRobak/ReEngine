#pragma once
#include "UIComponent.h"


class ControlPanel : public UIComponent
{
public:

    ControlPanel()
    {
    }

    void OnInit() override;

    void Render();
private:
    int type;
	ImTextureID startIcon = nullptr;
	ImTextureID pauseIcon = nullptr;
	ImTextureID recompileIcon = nullptr;
	bool isPlaying = false;
};

