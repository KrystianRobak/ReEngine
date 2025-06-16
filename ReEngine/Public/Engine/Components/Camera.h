#pragma once

#include <glm/glm.hpp>
#include "Transform.h"
#include <cmath>

#define myPi 3.14159265358979323846264338327950288

struct Camera
{
	Transform CameraTransform
	{
		glm::vec3(0.0f, 20.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
	};

	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	bool firstMouse = true;
	float yaw = -90.0f;
	float pitch = 0.0f;
	float lastX = 800.0f / 2.0;
	float lastY = 600.0 / 2.0;
	float fov = 90.0f;

	//void GenerateGUIElements(std::uint32_t entity)
	//{
 //       if (ImGui::CollapsingHeader(""))
 //       {
 //           // Add the "Delete" button
 //           std::string label = "Delete##" + entity;
 //           if (ImGui::Button(label.c_str(), ImVec2(-1, 0)))
 //           {
 //               
 //           }

 //           // Proceed with other GUI elements as before
 //           label = "Position##" + entity;
 //           ImGui::SliderFloat3(label.c_str(), &cameraPos[0], -15.f, 15.f);

 //           ImGui::BeginGroup();
 //           label = "AxisX" + name + std::to_string(index);
 //           ImGui::Checkbox(label.c_str(), &axis[0]);
 //           label = "AxisY" + name + std::to_string(index);
 //           ImGui::Checkbox(label.c_str(), &axis[1]);
 //           label = "AxisZ" + name + std::to_string(index);
 //           ImGui::EndGroup();

 //           label = "Angle " + name + std::to_string(index);
 //           ImGui::SliderFloat(label.c_str(), &angle, 0.f, 360.f);

 //           ImGui::BeginGroup();
 //           label = "Texture" + name + std::to_string(index);
 //           ImGui::Checkbox(label.c_str(), &texture);
 //           label = "Lightning" + name + std::to_string(index);
 //           ImGui::Checkbox(label.c_str(), &lightning);
 //           label = "Color Picker" + name + std::to_string(index);
 //           ImGui::ColorPicker4(label.c_str(), color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);
 //           ImGui::EndGroup();
 //       }
	//}
};


