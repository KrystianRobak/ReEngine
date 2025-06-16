#pragma once


#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <string>
#include <glm/glm.hpp>


struct Gravity
{
	glm::vec3 force;

    void GenerateGUIElements(std::uint32_t entity)
    {
        std::string entityStr = std::to_string(entity);

        std::string gravityLabel = "Gravity##" + entityStr;
        ImGui::SliderFloat3(gravityLabel.c_str(), &force[0], -15.f, 15.f);

    }
};
