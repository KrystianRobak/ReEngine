#pragma once
#include <memory>
#include "UIComponent.h"

#include <filesystem>
#include <string>

class SceneView : public UIComponent
{
public:
    SceneView() : size(800, 600)
    {


    }


    void resize(int32_t width, int32_t height);

    void Render() override;

private:
    glm::vec2 size;
};
