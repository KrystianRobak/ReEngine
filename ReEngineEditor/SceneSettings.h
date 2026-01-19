#pragma once

#include "UIComponent.h"
#include "ReScene.h"
#include "ReCamera.h"

class SceneSettings : public UIComponent
{
public:
	virtual void OnInit() override;
	virtual void Render() override;

private:
	Camera* camera;
};

