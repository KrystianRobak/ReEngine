#pragma once

#include "Window/FrameBuffer.h"
#include "RenderSystem.h"
#include "Commander.h" // <-- ADDED include for std::unique_ptr<Commander>
#include <memory>

struct Camera;

class IViewport
{
public:
    virtual ~IViewport() = default; // Added virtual destructor for base class

    // IViewport(int width, int height) = delete; // <-- REMOVED deleted constructor

    void SetCamera(Camera* cam) { camera = cam; }
    int32_t GetTexture() const { return framebuffer->get_texture(); }
    Commander* GetCommander() const { return commander.get(); }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    virtual void PreRender()
    {
        framebuffer->bind();
    }

    void Render(RenderSystem* renderer)
    {
        PreRender();
        renderer->RenderViewport(camera, commander.get());
        PostRender();
    }

    virtual void PostRender()
    {
        framebuffer->unbind();
    }

    // Added virtual Init for derived classes to call
    virtual void Init(int w, int h, RenderSystem* renderer)
    {
        this->width = w;
        this->height = h;
        framebuffer = renderer->CreateFramebuffer(width, height);
        commander = std::make_unique<Commander>();
        commander->Init(1024); // Assuming a default queue size
    }

protected:
    IViewport() = default; // <-- ADDED protected default constructor

    // Members moved to protected
    int width = 0;
    int height = 0;
    FrameBuffer* framebuffer = nullptr;
    Camera* camera = nullptr;
    std::unique_ptr<Commander> commander; // <-- CHANGED to unique_ptr
};