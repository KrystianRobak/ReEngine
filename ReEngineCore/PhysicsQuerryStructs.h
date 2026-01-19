#pragma once
#include <ReTypes.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

// PHYSICS QUERY SYSTEM: For raycasts, sphere casts, overlaps
struct RaycastHit {
    Entity entity = 0;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
    float distance = 0.0f;
};

struct PhysicsQueryParams {
    std::vector<std::string> channels;      // Filter by channel/layer (requires tag system)
    std::vector<Entity> ignoreEntities;     // Entities to skip
    bool includeStatic = true;              // Include static objects
    bool includeDynamic = true;             // Include dynamic objects
};
