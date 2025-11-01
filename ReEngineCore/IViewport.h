#pragma once

#include "Window/FrameBuffer.h""
#include "RenderSystem.h"
#include <memory>

class Commander;
class ReCamera;

class IViewport
{
public:
	IViewport(int width, int height) = delete;

    void SetCamera(ReCamera* cam) { camera = cam; }
    int32_t GetTexture() const { return framebuffer->get_texture(); }

	Commander* GetCommander() const { return commander; }

    virtual void PreRender() 
    {
        framebuffer->bind();
    }

    void Render(RenderSystem* renderer)
    {
        PreRender();

        renderer->RenderViewport(camera, commander);

        PostRender();
    }

    virtual void PostRender()
    {
		framebuffer->unbind();
    }

private:
    int width, height;
    std::unique_ptr<FrameBuffer> framebuffer;
    ReCamera* camera = nullptr;
	Commander* commander = nullptr;
};