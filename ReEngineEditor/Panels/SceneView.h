#pragma once

#include "UIComponent.h"
#include "Event.h"
#include "IViewport.h"

#include <filesystem>
#include <string>
#include "InputManagerApi.h"
#include "ReScene.h"

class SceneView : public UIComponent
{
public:
    SceneView() : size(800, 600)
    {

    }

    virtual void OnInit() override
    {
		viewport = engineApp->CreateNewViewport("SceneViewport", 1920, 1080);

        engineAPI->AddEventListener(Events::Application::CAMERA_CHANGED, [this](Event& e)
            {
                this->viewport->SetCamera(engineAPI->GetCurrentScene()->GetActiveCamera());
            });

		viewport->SetCamera(engineAPI->GetCurrentScene()->GetDefaultCamera()); 




        RotateIcon = GetTexture("icons/rotate.retex");
		TranslateIcon = GetTexture("icons/translate.retex");
		ScaleIcon = GetTexture("icson/scale.retex");

        IInputManager* input = engineApp->GetInputManager();

        // Movement Keys
        input->BindKey("Move Forward", 87);
        input->BindKey("Move Backward", 83);
        input->BindKey("Move Left", 65);
        input->BindKey("Move Right", 68);

        // Enable looking around when Right Mouse Button is held
        input->BindMouse("Enable Look", MouseButton::Right);
	}

    void resize(int32_t width, int32_t height);

    void PollInput(float currentDt);

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
