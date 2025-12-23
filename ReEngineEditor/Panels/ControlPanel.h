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
	std::shared_ptr<TextureResource> startIcon = nullptr;
    std::shared_ptr<TextureResource> pauseIcon = nullptr;
    std::shared_ptr<TextureResource> recompileIcon = nullptr;
	bool isPlaying = false;
};

