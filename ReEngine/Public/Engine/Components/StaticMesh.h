#pragma once

#include "ReflectionMacros.h"
#include <string>
#include <memory>
#include "ReTypes.h"

// Forward declaration to avoid including the heavy AssetManagerApi
struct MeshResource;

REFCOMPONENT()
struct StaticMesh
{
    // The path is useful for Serialization/Saving
    REFVARIABLE()
        std::string AssetPath;

    REFVARIABLE()
        int MaterialId = -1;

    // Runtime Handle. 
    // The RenderSystem checks MeshResource->uploaded to know if it can draw.
    std::shared_ptr<MeshResource> MeshResource = nullptr;
};