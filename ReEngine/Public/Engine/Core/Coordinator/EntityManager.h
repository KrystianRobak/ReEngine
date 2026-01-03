#pragma once
#include "ReTypes.h"
#include <array>
#include <cassert>
#include <queue>
#include <vector>

class EntityManager
{
public:
	EntityManager()
	{
		// Initialize the alive tracker
		mAliveEntities.resize(MAX_ENTITIES, false);

		// Fill the single pool with all available IDs
		for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
		{
			mAvailableEntities.push(entity);
		}
	}

	Entity CreateEntity()
	{
		assert(mLivingEntityCount < MAX_ENTITIES && "Too many entities in existence.");

		Entity id = mAvailableEntities.front();
		mAvailableEntities.pop();
		++mLivingEntityCount;

		mAliveEntities[id] = true; // Mark alive

		return id;
	}

	// Wrapper: No distinction in storage, just creates a standard entity
	Entity CreateLightEntity()
	{
		return CreateEntity();
	}

	void DestroyEntity(Entity entity)
	{
		assert(entity < MAX_ENTITIES && "Entity out of range.");

		// Safe guard: Do not destroy if already dead
		if (!mAliveEntities[entity])
		{
			return;
		}

		mSignatures[entity].reset();
		mAliveEntities[entity] = false; // Mark dead

		// Always return ID to the main pool
		mAvailableEntities.push(entity);
		--mLivingEntityCount;
	}

	// --- NEW: HARD RESET ---
	void Reset()
	{
		// 1. Reset counters
		mLivingEntityCount = 0;

		// 2. Clear the queue
		std::queue<Entity> empty;
		std::swap(mAvailableEntities, empty);

		// 3. Refill queue from 0 to MAX_ENTITIES to restore order (0, 1, 2...)
		for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
		{
			mAvailableEntities.push(entity);
		}

		// 4. Reset alive tracking
		std::fill(mAliveEntities.begin(), mAliveEntities.end(), false);

		// 5. Reset signatures
		for (auto& sig : mSignatures)
		{
			sig.reset();
		}
	}
	// -----------------------

	void SetSignature(Entity entity, Signature signature)
	{
		assert(entity < MAX_ENTITIES && "Entity out of range.");
		mSignatures[entity] = signature;
	}

	Signature GetSignature(Entity entity)
	{
		assert(entity < MAX_ENTITIES && "Entity out of range.");
		return mSignatures[entity];
	}

	std::uint32_t GetEntityCount()
	{
		return mLivingEntityCount;
	}

	std::uint32_t GetLightEntityCount()
	{
		return mLivingEntityCount;
	}

	void SetSelectedEntity(std::uint32_t entity)
	{
		this->selectedEntity = entity;
	}

	std::uint32_t GetSelectedEntity() {
		return this->selectedEntity;
	}

	bool IsAlive(Entity entity) const
	{
		if (entity >= MAX_ENTITIES) return false;
		return mAliveEntities[entity];
	}

private:
	std::queue<Entity> mAvailableEntities{};
	std::array<Signature, MAX_ENTITIES> mSignatures{};
	std::vector<bool> mAliveEntities; // Added for safety checks

	std::uint32_t mLivingEntityCount{};
	std::uint32_t selectedEntity = MAX_ENTITIES + 1;

	friend class SceneManager;
};