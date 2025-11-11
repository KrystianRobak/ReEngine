#pragma once

#include "ReICommand.h"
#include "Transform.h"
#include "ReTypes.h"
#include <cstdint>

struct RenderPrimitive {
    Entity entity;
    Transform transform;
    uint32_t mesh_id;
    uint32_t material_id;
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

