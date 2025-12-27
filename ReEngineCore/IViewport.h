#pragma once

#include "Window/FrameBuffer.h"
#include "RenderSystem.h"
#include <memory>

class Commander;
class Camera;

class IViewport
{
public:

    void SetCamera(Camera* cam) { camera = cam; }
    int32_t GetTexture() const { return framebuffer->get_texture(); }

	Commander* GetCommander() const { return commander; }

    virtual void PreRender() 
    {
        framebuffer->bind();
    }

    void Render(RenderSystem* renderer)
    {
        PreRender();

        renderer->RenderViewport(camera, commander, framebuffer.get());

        PostRender();
    }

    virtual void PostRender()
    {
		framebuffer->unbind();
    }

protected:
    std::string name;
    int width, height;
    std::unique_ptr<FrameBuffer> framebuffer;
    Camera* camera = nullptr;
	Commander* commander = nullptr;
};