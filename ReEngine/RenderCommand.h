#pragma once

#include "Engine/Components/Transform.h"

struct RenderPrimitive {
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

