#pragma once

#include "ReICommand.h"
#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#include "ReTypes.h"
#include <cstdint>

struct mTransform
{
	glm::vec3 position;

	glm::quat rotation;

	glm::vec3 scale;
};

struct RenderPrimitive {
    Entity entity;
    mTransform transform;
    uint32_t mesh_id;
    uint32_t material_id;
};


struct RenderCommand : public Command
{
    RenderCommand(int commandid, Entity entity, mTransform transform, uint32_t mesh_id, uint32_t material_id) : Command(commandid)
    {
        Primitive.entity = entity;
        Primitive.transform = transform;
        Primitive.material_id = material_id;
        Primitive.mesh_id = mesh_id;
    }

    RenderPrimitive Primitive;
};

