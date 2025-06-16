#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <glm/glm.hpp>

struct Thrust
{
	glm::vec3 force;

    void GenerateGUIElements(std::uint32_t entity)
    {
        std::string label = "Object##" + entity;
        if (ImGui::CollapsingHeader(label.c_str()))
        {
            label = "Thrust## " + entity;
            ImGui::SliderFloat3(label.c_str(), &force[0], -15.f, 15.f);
        }
    }
};