#pragma once

#include "UIComponent.h"
#include "Event.h"

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
        engineAPI->AddEventListener(Events::Engine::Renderer::RENDER_FINISHED, [&](Event& event)
            {
                this->SetTextureID(event.GetParam<uint64_t>("TextureId"));
            });
	}

    virtual void SetTextureID(uint64_t id)
    {
        this->textureID = id;
    }

    void resize(int32_t width, int32_t height);

    void Render() override;

private:
    uint64_t textureID;
    glm::vec2 size;
};
