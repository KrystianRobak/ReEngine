#pragma once

#include "ReflectionMacros.h"
#include <vector>
#include <memory>
#include <future>
#include <atomic>
#include "ReTypes.h"

class StaticMeshData;
class MeshResource;

struct PendingStaticMesh
{
    Entity entity;
    std::future<std::shared_ptr<StaticMeshData>> future;

    PendingStaticMesh() = default;

    // move-only
    PendingStaticMesh(PendingStaticMesh&&) noexcept = default;
    PendingStaticMesh& operator=(PendingStaticMesh&&) noexcept = default;

    // no copies
    PendingStaticMesh(const PendingStaticMesh&) = delete;
    PendingStaticMesh& operator=(const PendingStaticMesh&) = delete;
};


REFCOMPONENT()
struct StaticMesh
{
    REFVARIABLE()
    std::string AssetPath;

    REFVARIABLE()
    int MaterialId = -1;

    std::shared_ptr<MeshResource> MeshResource = nullptr;

    uint64_t MeshResourceId = 0;
};