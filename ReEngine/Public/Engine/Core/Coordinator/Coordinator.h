#pragma once

#include "ComponentManager.h"
#include "EntityManager.h"
#include "EventManager.h"
#include "SystemManager.h"
#include "ReTypes.h"
#include "Engine/Core/AnimationTypes.h"
#include "Engine/Components/Camera.h"
#include <memory>

#include "Engine/Systems/Animation/AnimationSequence.h"

#include "../ReEngineExport.h"

class ENGINE_API Coordinator
{
private:
	std::shared_ptr<ComponentManager> mComponentManager;
	std::unique_ptr<EntityManager> mEntityManager;
	std::unique_ptr<EventManager> mEventManager;
	std::unique_ptr<SystemManager> mSystemManager;

	Camera MainCamera;

	AnimationSequencer mReSequencer;
	ReSequencer Sequencer;

	static std::shared_ptr<Coordinator> instance;

public:

	static std::shared_ptr<Coordinator> GetCoordinator() 
	{
		if (instance == nullptr)
		{
			instance = std::make_shared<Coordinator>();
		}
		return instance;
	}

	Coordinator(): mReSequencer(&Sequencer) {
	}

	void Init()
	{
		mComponentManager = std::make_shared<ComponentManager>();
		mEntityManager = std::make_unique<EntityManager>();
		mEventManager = std::make_unique<EventManager>();
		mSystemManager = std::make_unique<SystemManager>();
	}

	std::shared_ptr<ComponentManager> GetComponentManager()
	{
		return this->mComponentManager;
	}  


	Entity CreateEntity()
	{
		return mEntityManager->CreateEntity();
	}

	Entity CreateLightEntity()
	{

		Entity entity = mEntityManager->CreateLightEntity();
		SendEvent(Events::Application::LIGHT_ENTITY_ADDED);
		return entity;
	}

	void DestroyEntity(Entity entity)
	{
		mEntityManager->DestroyEntity(entity);

		mComponentManager->EntityDestroyed(entity);

		mSystemManager->EntityDestroyed(entity);
	}

	std::uint32_t GetEntitiesAmount()
	{
		return mEntityManager->GetEntityCount();
	}

	std::uint32_t GetLightEntitiesAmount()
	{
		return mEntityManager->GetLightEntityCount();
	}

	void SetSelectedEntity(std::uint32_t entity) {
		this->mEntityManager->SetSelectedEntity(entity);
	}

	std::uint32_t GetSelectedEntity() {
		return this->mEntityManager->GetSelectedEntity();
	}

	Signature GetEntitySignature(Entity entity)
	{
		return mEntityManager->GetSignature(entity);
	}


	// Component methods
	void RegisterComponent(const Reflection::ClassInfo* classInfo)
	{
		mComponentManager->RegisterComponent(classInfo);
	}

	void AddComponent(Entity entity, const std::string& typeName, void* componentData)
	{
		mComponentManager->AddComponent(entity, typeName, componentData);

		auto signature = mEntityManager->GetSignature(entity);
		signature.set(mComponentManager->GetComponentType(typeName), true);
		mEntityManager->SetSignature(entity, signature);

		mSystemManager->EntitySignatureChanged(entity, signature);
	}

	void RemoveComponent(Entity entity, const std::string& typeName)
	{
		mComponentManager->RemoveComponent(entity, typeName);

		auto signature = mEntityManager->GetSignature(entity);
		signature.set(mComponentManager->GetComponentType(typeName), false);
		mEntityManager->SetSignature(entity, signature);

		mSystemManager->EntitySignatureChanged(entity, signature);
	}

	// Returns a raw pointer, which the caller must cast
	void* GetComponent(Entity entity, const std::string& typeName)
	{
		return mComponentManager->GetComponent(entity, typeName);
	}

	ComponentType GetComponentType(const std::string& typeName)
	{
		return mComponentManager->GetComponentType(typeName);
	}

	std::unordered_map<const char*, ComponentType> GetComponentsTypes()
	{
		return mComponentManager->GetComponentsTypes();
	}


	// System methods
	System* RegisterSystem(const Reflection::ClassInfo* classInfo)
	{
		// Directly forwards the reflection data to the SystemManager.
		return mSystemManager->RegisterSystem(classInfo);
	}


	System* GetSystem(const std::string& typeName)
	{
		// Asks the SystemManager for a system by its string name.
		return mSystemManager->GetSystem(typeName);
	}


	void SetSystemSignature(const std::string& typeName, Signature signature)
	{
		// Tells the SystemManager to set the signature for the named system.
		mSystemManager->SetSignature(typeName, signature);
	}


	// Event methods
	void AddEventListener(EventType eventType, std::function<void(Event&)> const& listener)
	{
		mEventManager->AddListener(eventType, listener);
	}

	void SendEvent(Event& event)
	{
		mEventManager->SendEvent(event);
	}

	void SendEvent(EventType eventType)
	{
		mEventManager->SendEvent(eventType);
	}

	Camera* GetCamera()
	{
		return &MainCamera;
	}

	AnimationSequencer* GetScene() {
		return &mReSequencer;
	}
};
