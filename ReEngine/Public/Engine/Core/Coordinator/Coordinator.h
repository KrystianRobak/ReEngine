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
	std::shared_ptr<EventManager> mEventManager;
	std::shared_ptr<SystemManager> mSystemManager;
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
		mEventManager = std::make_shared<EventManager>();
		mSystemManager = std::make_shared<SystemManager>();
		mAssetManager = std::make_shared<AssetManager>(pool);
		mSceneManager = std::make_shared<SceneManager>();
		mEpochManager = std::make_shared<EpochManager>();
	}

	std::shared_ptr<ComponentManager> GetComponentManager()
	{
		return this->mComponentManager;
	}  

	void PrepareForReload() override
	{
		LOGF_INFO("[Coordinator] Preparing for hot-reload...");

		LOGF_INFO("[Coordinator] Backing up scene for reload...");

		LOGF_INFO("[Coordinator] Clearing managers...");
		ClearForReload();

		LOGF_INFO("[Coordinator] Ready for DLL reload.");
	}

	void RestoreAfterReload() override
	{
		LOGF_INFO("[Coordinator] Restoring after hot-reload...");
	}

	void ClearForReload() override
	{
		LOGF_INFO("[Coordinator] Clearing scene...");
		ClearScene();

		LOGF_INFO("[Coordinator] Unregistering GameModule systems...");
		auto systems = Reflection::Registry::Instance().GetAllSystems();
		for (auto& sys : systems) {
			if (std::string(sys->module) == "GameModule") {
				mSystemManager->UnregisterSystem(sys->fullName);
			}
		}

		LOGF_INFO("[Coordinator] Unregistering GameModule components...");
		auto components = Reflection::Registry::Instance().GetAllComponents();
		for (auto& comp : components) {
			if (std::string(comp->module) == "GameModule") {
				mComponentManager->UnregisterComponent(comp->fullName);
			}
		}

		LOGF_INFO("[Coordinator] GameModule cleanup complete. Core systems preserved.");
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

	System* RegisterSystem(const Reflection::ClassInfo* classInfo) override
	{
		return mSystemManager->RegisterSystem(classInfo);
	}


	System* GetSystem(const std::string& typeName) override
	{
		return mSystemManager->GetSystem(typeName);
	}

	void OnBeginSimulation()
	{
		mSystemManager->OnBeginSimulation();
	}

	void OnEndSimulation()
	{
		mSystemManager->OnEndSimulation();
	}


	void SetSystemSignature(const std::string& typeName, Signature signature) override
	{
		mSystemManager->SetSignature(typeName, signature);
	}

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
		std::cout << "[Coordinator] Entering Play Mode: Backing up scene..." << std::endl;
		SaveScene(PlayModeBackupPath);


	}

	void ExitPlayMode()
	{
		std::cout << "[Coordinator] Exiting Play Mode: Restoring scene..." << std::endl;

		ClearScene();

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

		for (Entity i = 0; i < MAX_ENTITIES; ++i)
		{

			if (mEntityManager->IsAlive(i))
			{
				DestroyEntity(i);
			}
		}
		ProcessPendingEntityDeletions();

		mEntityManager->Reset();

		SetSelectedEntity(MAX_ENTITIES + 1);
	}

	ReScene* GetCurrentScene() override
	{
		return mSceneManager->currentScene_;
	}

	void OpenScene(const std::string& path) override
	{
		ClearScene();
		ReScene* scene = mSceneManager->LoadScene(path, mEntityManager, this, true);
		scene->SetPath(path);

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

		ComponentType typeId = mComponentManager->GetComponentType(componentName);

		for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
		{
			if (mEntityManager->IsAlive(entity))
			{
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
