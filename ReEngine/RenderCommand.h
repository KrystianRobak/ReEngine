#pragma once

#include "Transform.h"

#include "ReTypes.h"
#include <cstdint>

struct RenderPrimitive {
    Entity entity;
    Transform transform;
    uint32_t mesh_id;
    uint32_t material_id;
};

class RenderCommand
{
private:
    
    uint32_t CommandId;
	RenderPrimitive Primitive;


public:
    
    RenderCommand(uint32_t Id, RenderPrimitive primitive);

	uint32_t GetCommandId() const { return CommandId; }
    
	RenderPrimitive GetRenderPrimitive() const { return Primitive; }

};

