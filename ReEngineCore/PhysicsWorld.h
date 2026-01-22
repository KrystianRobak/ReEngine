#pragma once

#include "PhysicsQuerryStructs.h"


class PhysicsWorld
{
public:
    virtual bool RaycastSingle(const glm::vec3& origin, const glm::vec3& direction, float maxDistance,
        RaycastHit& outHit, const PhysicsQueryParams& params = PhysicsQueryParams()) = 0;

    virtual std::vector<RaycastHit> RaycastAll(const glm::vec3& origin, const glm::vec3& direction,
        float maxDistance, const PhysicsQueryParams& params = PhysicsQueryParams()) = 0;

    virtual bool SphereCastSingle(const glm::vec3& origin, float radius, const glm::vec3& direction,
        float maxDistance, RaycastHit& outHit, const PhysicsQueryParams& params = PhysicsQueryParams()) = 0;

    virtual std::vector<RaycastHit> SphereCastAll(const glm::vec3& origin, float radius,
        const glm::vec3& direction, float maxDistance, const PhysicsQueryParams& params = PhysicsQueryParams()) = 0;

    virtual std::vector<Entity> OverlapSphere(const glm::vec3& center, float radius,
        const PhysicsQueryParams& params = PhysicsQueryParams()) = 0;

    virtual std::vector<Entity> OverlapBox(const glm::vec3& center, const glm::vec3& halfExtents,
        const glm::quat& rotation = glm::quat(1, 0, 0, 0),
        const PhysicsQueryParams& params = PhysicsQueryParams()) = 0;
    
};