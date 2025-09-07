#pragma once

#include "BaseApi.h"
#include <ReTypes.h>

class System;

namespace Editor
{
	class IEngineEditorApi
	{
	public:

		virtual Entity CreateEntity() = 0;

		virtual Entity CreateLightEntity() = 0;

		virtual void DestroyEntity(Entity entity) = 0;

		virtual std::uint32_t GetEntitiesAmount() = 0;

		virtual std::uint32_t GetLightEntitiesAmount() = 0;

		virtual void SetSelectedEntity(std::uint32_t entity) = 0;

		virtual std::uint32_t GetSelectedEntity() = 0;

		virtual Signature GetEntitySignature(Entity entity) = 0;

		virtual void AddComponent(Entity entity, const std::string& fullName) = 0;

		virtual void RemoveComponent(Entity entity, const std::string& typeName) = 0;

		virtual void SwapComponentBuffers(const std::string& fullName) = 0;

		//virtual void AddEventListener(EventType eventType, std::function<void(Event&)> const& listener) = 0;

		//virtual void SendEvent(Event& event) = 0;

		virtual void* GetComponent(Entity entity, const std::string& typeName) = 0;

		virtual System* GetSystem(const std::string& typeName) = 0;

		virtual ComponentType GetComponentType(const std::string& typeName) = 0;

		virtual void SendEvent(EventType eventType) = 0;
	};
}