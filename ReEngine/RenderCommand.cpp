#include "RenderCommand.h"

RenderCommand::RenderCommand(uint32_t Id, RenderPrimitive primitive)
{
	CommandId = Id;
	Primitive = primitive;
}
