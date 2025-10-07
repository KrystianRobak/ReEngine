#pragma once

#include "UIComponent.h"
#include "ReScene.h"

class SceneSettings : public UIComponent
{
public:
	virtual void OnInit() override;
	virtual void Render() override;
};

