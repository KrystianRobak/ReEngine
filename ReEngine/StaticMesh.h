#pragma once

#include "ReflectionMacros.h"
#include <vector>
#include <memory>

class StaticMeshData;

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
	int StaticMeshId = 0;
	std::shared_ptr<StaticMeshData> StaticMeshHandler;
};