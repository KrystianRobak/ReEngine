#pragma once

#include "ReICommand.h"
#include "Transform.h"
#include "ReTypes.h"
#include <cstdint>
#include <memory>

class MeshResource;

struct RenderPrimitive {
    glm::mat4 ModelMatrix;
    Entity Entity;
    int MaterialId;

    // Direct handle to the mesh resource (including VAO and uploaded flag)
    std::shared_ptr<MeshResource> Mesh;
    std::vector<glm::mat4> FinalBoneMatrices;
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

