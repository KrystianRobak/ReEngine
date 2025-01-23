#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <string>
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform
{
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;

    void GenerateGUIElements(std::uint32_t entity)
    {
        std::string entityStr = std::to_string(entity);

        std::string transformLabel = "Transform##" + entityStr;
        ImGui::Text("%s", transformLabel.c_str());

        std::string positionLabel = "Position##" + entityStr;
        ImGui::SliderFloat3(positionLabel.c_str(), &position[0], -30.f, 30.f);

    	glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(rotation));
    	std::string rotationLabel = "Rotation##" + entityStr;
    	if (ImGui::SliderFloat3(rotationLabel.c_str(), &eulerAngles[0], -180.f, 180.f)) {
    		rotation = glm::quat(glm::radians(eulerAngles)); // Update quaternion
    	}

        std::string scaleLabel = "Scale##" + entityStr;
        ImGui::SliderFloat3(scaleLabel.c_str(), &scale[0], -15.f, 15.f);
    }
};
