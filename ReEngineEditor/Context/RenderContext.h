#pragma once

#include "../IWindow.h"

class RenderContext
{

public:

    RenderContext() : window(nullptr) {}

    virtual bool init(IWindow* window)
    {
        this->window = window;
        return true;
    }

    virtual void pre_render() = 0;

    virtual void post_render() = 0;

    virtual void end() = 0;

protected:
    IWindow* window;
};

