#pragma once

#include "Api/BaseApi.h"
#include "Api/AssetManagerApi.h"
#include <memory>
#include <ReTypes.h>

class Event;
class System;
class ReScene;

namespace Editor
{
	class IEngineEditorApi
	{
	public:

		virtual std::shared_ptr<AssetManagerApi> GetAssetManager() = 0;

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

		virtual void MarkEntityDirty(Entity entity, const std::string& typeName) = 0;

		virtual void SwapComponentBuffers(const std::string& fullName) = 0;

		virtual void AddEventListener(EventType, std::function<void(Event&)> const& listener) = 0;

		virtual void SendEvent(Event& event) = 0;

		virtual void SendEvent(EventType eventType) = 0;

		virtual ReScene* GetCurrentScene() = 0;

		virtual void* GetComponent(Entity entity, const std::string& typeName) = 0;

		virtual System* GetSystem(const std::string& typeName) = 0;

		virtual ComponentType GetComponentType(const std::string& typeName) = 0;

		
	};
}