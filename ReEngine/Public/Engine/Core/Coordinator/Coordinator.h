#pragma once

#include "Api/SystemApi/CoordinatorSystemApi.h"
#include "Api/EngineApi/CoordinatorEditorApi.h"

#include "ComponentManager.h"
#include "EntityManager.h"
#include "EventManager.h"
#include "SystemManager.h"
#include "AssetManager.h"
#include "SceneManager.h"
#include "EpochManager.h"

#include "ReTypes.h"
#include "Engine/Core/AnimationTypes.h"
#include <memory>

#include "Engine/Systems/Animation/AnimationSequence.h"

#include "ReEngineExport.h"


class ENGINE_API Coordinator : public Editor::IEngineEditorApi
{
private:
	std::shared_ptr<ComponentManager> mComponentManager;
	std::shared_ptr<EntityManager> mEntityManager;
	std::unique_ptr<EventManager> mEventManager;
	std::unique_ptr<SystemManager> mSystemManager;
	std::shared_ptr<AssetManager> mAssetManager;
	std::shared_ptr<SceneManager> mSceneManager;
	std::shared_ptr<EpochManager> mEpochManager;

	Camera MainCamera;

	const std::string PlayModeBackupPath = "Temp/PlayModeBackup.scene";
	std::set<Entity> PendingEntityDeletions;

	AnimationSequencer mReSequencer;
	ReSequencer Sequencer;

	static std::shared_ptr<Coordinator> instance;

public:

	std::shared_ptr<AssetManagerApi> GetAssetManager() override
	{
		return std::static_pointer_cast<AssetManagerApi>(mAssetManager);
	}

	std::shared_ptr<EpochManager> GetEpochManager()
	{
		return mEpochManager;
	}

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


	void Init(ThreadPool* pool)
	{
		mComponentManager = std::make_shared<ComponentManager>();
		mEntityManager = std::make_shared<EntityManager>();
		mEventManager = std::make_unique<EventManager>();
		mSystemManager = std::make_unique<SystemManager>();
		mAssetManager = std::make_shared<AssetManager>(pool);
		mSceneManager = std::make_shared<SceneManager>();
		mEpochManager = std::make_shared<EpochManager>();
	}

	std::shared_ptr<ComponentManager> GetComponentManager()
	{
		return this->mComponentManager;
	}  


	Entity CreateEntity() override
	{
		return mEntityManager->CreateEntity();
	}

	Entity CreateLightEntity() override
	{

		Entity entity = mEntityManager->CreateLightEntity();
		SendEvent(Events::Application::LIGHT_ENTITY_ADDED);
		return entity;
	}

	void ScheduleEntityDestruction(Entity entity)
	{
		PendingEntityDeletions.insert(entity);
	}

	void ProcessPendingEntityDeletions()
	{
		for (Entity entity : PendingEntityDeletions)
		{
			DestroyEntity(entity);
		}
		PendingEntityDeletions.clear();
	}

	void DestroyEntity(Entity entity) override
	{
		if (!mEntityManager->IsAlive(entity))
		{
			return;
		}

		mEntityManager->DestroyEntity(entity);

		mComponentManager->EntityDestroyed(entity);

		mSystemManager->EntityDestroyed(entity);

		mSceneManager->EntityDestroyed(entity);
	}

	bool IsEntityAlive(Entity entity) override
	{
		return mEntityManager->IsAlive(entity);
	}

	std::uint32_t GetEntitiesAmount() override
	{
		return mEntityManager->GetEntityCount();
	}

	std::uint32_t GetLightEntitiesAmount() override
	{
		return mEntityManager->GetLightEntityCount();
	}

	void SetSelectedEntity(std::uint32_t entity) override 
	{
		this->mEntityManager->SetSelectedEntity(entity);
	}

	std::uint32_t GetSelectedEntity() override 
	{
		return this->mEntityManager->GetSelectedEntity();
	}

	Signature GetEntitySignature(Entity entity) override
	{
		return mEntityManager->GetSignature(entity);
	}


	// Component methods
	void RegisterComponent(const Reflection::ClassInfo* classInfo, bool IsDoubleBuffered = false) override
	{
		mComponentManager->RegisterComponent(classInfo, IsDoubleBuffered);
	}

	void AddComponent(Entity entity, const std::string& fullName) override
	{
		auto componentInfo = Reflection::Registry::Instance().FindComponent(fullName);
		void* componentData = componentInfo->construct();
		mComponentManager->AddComponent(entity, fullName, componentData);

		auto signature = mEntityManager->GetSignature(entity);
		signature.set(mComponentManager->GetComponentType(fullName), true);
		mEntityManager->SetSignature(entity, signature);

		mSystemManager->EntitySignatureChanged(entity, signature);
	}

	bool HasComponent(Entity entity, const char* name) {
		Signature signature = GetEntitySignature(entity);
		if (signature.test(GetComponentType(name)))
		{
			return true;
		}
		return false;
	}

	void RemoveComponent(Entity entity, const std::string& typeName) override
	{
		mComponentManager->RemoveComponent(entity, typeName);

		auto signature = mEntityManager->GetSignature(entity);
		signature.set(mComponentManager->GetComponentType(typeName), false);
		mEntityManager->SetSignature(entity, signature);

		mSystemManager->EntitySignatureChanged(entity, signature);
	}

	void MarkEntityDirty(Entity entity, const std::string& typeName) override
	{
		mComponentManager->MarkEntityDirty(entity, typeName);
	}

	void* GetComponentForWrite(Entity entity, const std::string& typeName) override
	{
		return mComponentManager->GetComponentForWrite(entity, typeName);
	}

	void SwapComponentBuffers(const std::string& fullName) {
		mComponentManager->SwapComponentBuffers(fullName);
	}

	// Returns a raw pointer, which the caller must cast
	void* GetComponent(Entity entity, const std::string& typeName) override
	{
		return mComponentManager->GetComponent(entity, typeName);
	}

	ComponentType GetComponentType(const std::string& typeName) override
	{
		return mComponentManager->GetComponentType(typeName);
	}

	std::unordered_map<std::string, ComponentType> GetComponentsTypes()
	{
		return mComponentManager->GetComponentsTypes();
	}

	std::vector<char>* GetWriteBuffer(const std::string& typeName)
	{
		return mComponentManager->GetComponentWriteBuffer(typeName);
	}

	std::vector<char>* GetReadBuffer(const std::string& typeName)
	{
		return mComponentManager->GetComponentReadBuffer(typeName);
	}


	// System methods
	System* RegisterSystem(const Reflection::ClassInfo* classInfo) override
	{
		// Directly forwards the reflection data to the SystemManager.
		return mSystemManager->RegisterSystem(classInfo);
	}


	System* GetSystem(const std::string& typeName) override
	{
		// Asks the SystemManager for a system by its string name.
		return mSystemManager->GetSystem(typeName);
	}


	void SetSystemSignature(const std::string& typeName, Signature signature) override
	{
		// Tells the SystemManager to set the signature for the named system.
		mSystemManager->SetSignature(typeName, signature);
	}


	// Event methods
	void AddEventListener(EventType eventType, std::function<void(Event&)> const& listener) override
	{
		mEventManager->AddListener(eventType, listener);
	}

	void SendEvent(Event& event) override
	{
		mEventManager->SendEvent(event);
	}

	void SendEvent(EventType eventType) override
	{
		mEventManager->SendEvent(eventType);
	}

	//SceneManager

	// --- Play Mode Logic ---

	Entity InstantiatePrefab(const std::string& path)
	{
		return mSceneManager->InstantiatePrefab(path, mEntityManager, this);
	}

	bool SaveEntityAsPrefab(Entity entity, const std::string& path)
	{
		return mSceneManager->SaveAsPrefab(entity, path, mEntityManager, this);
	}

	void EnterPlayMode()
	{
		// 1. Save the current state to a temp file
		std::cout << "[Coordinator] Entering Play Mode: Backing up scene..." << std::endl;
		SaveScene(PlayModeBackupPath);


	}

	void ExitPlayMode()
	{
		std::cout << "[Coordinator] Exiting Play Mode: Restoring scene..." << std::endl;
		// 1. Clear the simulation state (destroy all entities)
		ClearScene();

		// 2. Load the original state
		if (std::filesystem::exists(PlayModeBackupPath))
		{
			OpenScene(PlayModeBackupPath);
		}
		else
		{
			std::cerr << "[Coordinator] Error: Backup scene file not found!" << std::endl;
		}
	}

	void ClearScene()
	{
		// Iterate 0 to MAX_ENTITIES safely
		for (Entity i = 0; i < MAX_ENTITIES; ++i)
		{
			// Safely check if entity is valid before destroying
			if (mEntityManager->IsAlive(i))
			{
				DestroyEntity(i);
			}
		}
		// Flush any pending removals immediately
		ProcessPendingEntityDeletions();

		// 2. HARD RESET the EntityManager.
		// This resets the ID counter to 0 and clears the internal queue
		// so the next created entity is guaranteed to be 0 (or the first available).
		mEntityManager->Reset();

		// Reset selection
		SetSelectedEntity(MAX_ENTITIES + 1);
	}

	ReScene* GetCurrentScene() override
	{
		return mSceneManager->currentScene_;
	}

	void OpenScene(const std::string& path) override
	{
		mSceneManager->LoadScene(path, mEntityManager, this, true);


		SetSelectedEntity(MAX_ENTITIES + 1);
	}

	void SaveScene(const std::string& path)
	{
		mSceneManager->SerializeScene(path, mEntityManager, this);
	}

	Camera* GetCamera()
	{
		return &MainCamera;
	}

	AnimationSequencer* GetScene() {
		return &mReSequencer;
	}

	std::vector<Entity> GetEntitiesWith(const std::string& componentName)
	{
		std::vector<Entity> matchingEntities;

		// 1. Get the ID for the requested component
		// Note: Ensure componentName is valid/registered, or this might assert inside GetComponentType
		ComponentType typeId = mComponentManager->GetComponentType(componentName);

		// 2. Iterate through all potential entities
		for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
		{
			// Check if entity is actually in use
			if (mEntityManager->IsAlive(entity))
			{
				// 3. Check if the entity's signature has the specific bit set
				Signature sig = mEntityManager->GetSignature(entity);
				if (sig.test(typeId))
				{
					matchingEntities.push_back(entity);
				}
			}
		}

		return matchingEntities;
	}
};
