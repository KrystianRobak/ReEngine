#pragma once
#include <glm/glm.hpp>

enum class LightType {
    Directional,
    Point,
    Spot
};

inline std::string GetLightType(const LightType type) {
    switch (type)
    {
        case LightType::Directional:
            return "Directional";
        case LightType::Point:
            return "Point";
        case LightType::Spot:
            return "Spot";
    }

    return "Null";
}

struct LightSource {
    int type = static_cast<int>(LightType::Directional);
    glm::vec3 LightColor;
    float intensity;

    glm::vec3 Ambient;
    glm::vec3 Diffuse;
    glm::vec3 Specular;

    // Additional properties like attenuation for point lights
    float constant;
    float linear;
    float quadratic;
    // For spotlights
    float cutOff;
    float outerCutOff;

    void GenerateGUIElements(std::int32_t entity)
    {
        const char* items[] = {
            GetLightType(LightType::Directional).c_str(),
            GetLightType(LightType::Point).c_str(),
            GetLightType(LightType::Spot).c_str()
        };

        std::string entityStr = std::to_string(entity);
        if (ImGui::Combo("Select Option", &type, items, 3)) {
            printf("Selected option: %s\n", items[type]);
        }


        if (type == static_cast<int>(LightType::Directional))
        {
            std::string colorLabel = "Color##" + entityStr;
            ImGui::ColorPicker4(colorLabel.c_str(), &LightColor[0], ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB);

            std::string shineLabel = "Intensity##" + entityStr;
            ImGui::SliderFloat(shineLabel.c_str(), &intensity, 0.f, 1.f);

            std::string positionLabel = "Ambient##" + entityStr;
            ImGui::SliderFloat3(positionLabel.c_str(), &Ambient[0], 0.f, 1.f);

            std::string rotationLabel = "Diffuse##" + entityStr;
            ImGui::SliderFloat3(rotationLabel.c_str(), &Diffuse[0], 0.f, 1.f);

            std::string scaleLabel = "Specular##" + entityStr;
            ImGui::SliderFloat3(scaleLabel.c_str(), &Specular[0], 0.f, 1.f);


        }
        else if (type == static_cast<int>(LightType::Point))
        {
            std::string entityStr = std::to_string(entity);
            std::string colorLabel = "Color##" + entityStr;

        }
        else if (type == static_cast<int>(LightType::Spot)) {
            std::string entityStr = std::to_string(entity);
            std::string colorLabel = "Color##" + entityStr;
        }

    };
};
