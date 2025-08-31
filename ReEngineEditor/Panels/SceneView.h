#pragma once
#include <memory>
#include "Context/OpenGlFrameBuffer.h"
#include "UIComponent.h"
#include "Engine/Core/Application.h"
#include <filesystem>
#include <string>

class SceneView : public UIComponent
{
public:
    SceneView() : frameBuffer(nullptr), size(800, 600)
    {
        frameBuffer = std::make_unique<OpenGlFrameBuffer>();
        frameBuffer->create_buffers(800, 600);
       // mLight = std::make_unique<nelems::Light>();
        //std::cout << application << std::endl;
    }


    void resize(int32_t width, int32_t height);

    virtual void Render();

	void PreRender();

    void PostRender();

private:
    std::unique_ptr<OpenGlFrameBuffer> frameBuffer;
    glm::vec2 size;
};
