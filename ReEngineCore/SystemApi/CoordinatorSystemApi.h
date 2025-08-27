#pragma once

#include <ReTypes.h>



namespace Engine
{
	class IEngineApi 
	{
	public:
		virtual void* GetComponent(Entity entity, const std::string& typeName) = 0;
	};
}