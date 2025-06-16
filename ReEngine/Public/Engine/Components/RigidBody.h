#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <glm/glm.hpp>

struct RigidBody
{
    float mass = 3;
	glm::vec3 velocity;
	glm::vec3 acceleration;
    glm::vec3 momentum;
    bool repositioned = false;

    void GenerateGUIElements(std::uint32_t entity)
    {
        std::string entityStr = std::to_string(entity);

        std::string velocityLabel = "Velocity##" + entityStr;
        ImGui::SliderFloat3(velocityLabel.c_str(), &velocity[0], -20.f, 20.f);

        std::string massLabel = "Mass##" + entityStr;
        ImGui::InputFloat(massLabel.c_str(), &mass);

        std::string accelerationLabel = "Acceleration##" + entityStr;
        ImGui::SliderFloat3(accelerationLabel.c_str(), &acceleration[0], -15.f, 15.f);
    }
};
