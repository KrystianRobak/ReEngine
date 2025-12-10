#pragma once

#include "ReICommand.h"
#include "Transform.h"
#include "ReTypes.h"
#include <cstdint>

struct RenderPrimitive {
    uint64_t MeshResourceId;     // GPU resource lookup
    glm::mat4 ModelMatrix;       // snapshot, no pointer to Transform
    uint32_t Entity;
	uint64_t MaterialId;    // Material to use for rendering
};

struct RenderCommand : public Command
{
    RenderCommand(uint32_t id, const RenderPrimitive& primitive)
    {
        commandId = id;
        Primitive = primitive;
	}

    RenderPrimitive Primitive;
};

