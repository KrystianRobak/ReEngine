#pragma once

#include "ComponentManager.h"
#include "EntityManager.h"
#include "EventManager.h"
#include "SystemManager.h"
#include "Engine/Core/Types.h"
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
	template<typename T>
	void RegisterComponent()
	{
		mComponentManager->RegisterComponent<T>();
	}

	template<typename T>
	void AddComponent(Entity entity, T component)
	{
		mComponentManager->AddComponent<T>(entity, component);

		auto signature = mEntityManager->GetSignature(entity);
		signature.set(mComponentManager->GetComponentType<T>(), true);
		mEntityManager->SetSignature(entity, signature);

		mSystemManager->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	void RemoveComponent(Entity entity)
	{
		mComponentManager->RemoveComponent<T>(entity);

		auto signature = mEntityManager->GetSignature(entity);
		signature.set(mComponentManager->GetComponentType<T>(), false);
		mEntityManager->SetSignature(entity, signature);

		mSystemManager->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	T& GetComponent(Entity entity)
	{
		return mComponentManager->GetComponent<T>(entity);
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		return mComponentManager->GetComponentType<T>();
	}

	std::unordered_map<const char*, ComponentType> GetComponentsTypes()
	{
		return mComponentManager->GetComponentsTypes();
	}


	// System methods
	template<typename T>
	std::shared_ptr<T> RegisterSystem()
	{
		return mSystemManager->RegisterSystem<T>();
	}

	template<typename T>
	std::shared_ptr<T> GetSystem()
	{
		return mSystemManager->GetSystem<T>();
	}


	template<typename T>
	void SetSystemSignature(Signature signature)
	{
		mSystemManager->SetSignature<T>(signature);
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
