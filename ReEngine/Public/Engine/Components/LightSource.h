#pragma once

#include "ReflectionMacros.h"
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

REFCOMPONENT()
struct LightSource {
    REFVARIABLE()
        int type = static_cast<int>(LightType::Directional);

    REFVARIABLE()
        glm::vec3 LightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    REFVARIABLE()
        float intensity = 1.0f;

    REFVARIABLE()
        glm::vec3 Ambient = glm::vec3(0.1f);
    REFVARIABLE()
        glm::vec3 Diffuse = glm::vec3(1.0f);
    REFVARIABLE()
        glm::vec3 Specular = glm::vec3(1.0f);

    REFVARIABLE() float constant = 1.0f;
    REFVARIABLE() float linear = 0.09f;
    REFVARIABLE() float quadratic = 0.032f;

    REFVARIABLE() float cutOff = glm::cos(glm::radians(12.5f));
    REFVARIABLE() float outerCutOff = glm::cos(glm::radians(15.0f));
};
