#pragma once
#include "RenderContext.h"

class UIContext : public RenderContext
{

public:

    bool init(IWindow* window) override;

    void pre_render() override;

    void post_render() override;

    void end() override;

};

