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
		viewport = engineApp->CreateNewViewport("SceneViewport", 1920, 1080);


        RotateIcon = GetTexture("pngs/rotate.jpg");
		TranslateIcon = GetTexture("pngs/translate.jpg");
		ScaleIcon = GetTexture("pngs/scale.jpg");
	}

    void resize(int32_t width, int32_t height);

    void Render() override;

private:
    glm::vec2 size;

	IViewport* viewport = nullptr;

    std::shared_ptr<TextureResource> TranslateIcon;
    std::shared_ptr<TextureResource> RotateIcon;
    std::shared_ptr<TextureResource> ScaleIcon;

    // State for the pop-up
    bool showImportTypePopup = false;

    // Data passed from the drag-drop event
    std::string pendingImportPath;
    std::string pendingImportName;
};
