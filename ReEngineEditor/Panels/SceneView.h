#pragma once

#include "UIComponent.h"
#include "Event.h"
#include "IViewport.h"

#include <filesystem>
#include <string>

class SceneView : public UIComponent
{
public:
    SceneView() : size(800, 600)
    {

    }

    virtual void OnInit() override
    {
		viewport = engineApp->CreateNewViewport("SceneViewport");


        RotateIcon = LoadTexture("pngs/rotate.jpg");
		TranslateIcon = LoadTexture("pngs/translate.jpg");
		ScaleIcon = LoadTexture("pngs/scale.jpg");
	}

    void resize(int32_t width, int32_t height);

    void Render() override;

private:
    glm::vec2 size;

	IViewport* viewport = nullptr;

	ImTextureID TranslateIcon;
	ImTextureID RotateIcon;
	ImTextureID ScaleIcon;
};
