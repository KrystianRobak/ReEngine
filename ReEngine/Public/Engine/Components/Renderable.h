#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glm/glm.hpp>

struct Renderable
{
	glm::vec3 color;
    glm::vec3 Ambient;
    glm::vec3 Diffuse;
    glm::vec3 Specular;
    float Shininess;

    void GenerateGUIElements(std::uint32_t entity)
    {
        std::string entityStr = std::to_string(entity);

        std::string colorLabel = "Color##" + entityStr;
        ImGui::ColorPicker4(colorLabel.c_str(), &color[0], ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);

        std::string positionLabel = "Ambient##" + entityStr;
        ImGui::SliderFloat3(positionLabel.c_str(), &Ambient[0], -0.f, 1.f);

        std::string rotationLabel = "Diffuse##" + entityStr;
        ImGui::SliderFloat3(rotationLabel.c_str(), &Diffuse[0], 0.f, 1.f);

        std::string scaleLabel = "Specular##" + entityStr;
        ImGui::SliderFloat3(scaleLabel.c_str(), &Specular[0], 0.f, 1.f);

        std::string shineLabel = "Shineness##" + entityStr;
        ImGui::SliderFloat(shineLabel.c_str(), &Shininess, 0.f, 1.f);
    }
};
