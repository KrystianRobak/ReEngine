#pragma once
#include "ReflectionMacros.h"
#include "Collision/AABB.h"
#include "StaticMeshData.h"

REFCOMPONENT()
struct MeshCollider {
    std::shared_ptr<StaticMeshData> meshData;
    std::shared_ptr<AABB> aabb;
    MeshCollider() { aabb = std::make_shared<AABB>(); }

    void Update(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) {
        if (meshData) {
            aabb->SetLocalBounds(meshData->aabbMin, meshData->aabbMax);
            aabb->UpdateWorldTransform(pos, rot, scale);
        }
    }
};