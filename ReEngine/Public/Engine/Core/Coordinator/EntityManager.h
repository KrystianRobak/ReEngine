#pragma once
#include "ReTypes.h"
#include <array>
#include <cassert>
#include <queue>


class EntityManager
{
public:
	EntityManager()
	{
		for (Entity entity = 0; entity < MAX_LIGHT_ENTITIES; entity++)
		{
			mAvailableLightEntities.push(entity);
		}

		for (Entity entity = MAX_LIGHT_ENTITIES+1; entity < MAX_ENTITIES; ++entity)
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

		return id;
	}

	Entity CreateLightEntity()
	{
		assert(mLivingLightEntityCount < MAX_LIGHT_ENTITIES && "Too many entities in existence.");

		Entity id = mAvailableLightEntities.front();
		mAvailableLightEntities.pop();
		++mLivingLightEntityCount;

		return id;
	}

	void DestroyEntity(Entity entity)
	{
		assert(entity < MAX_ENTITIES && "Entity out of range.");

		mSignatures[entity].reset();
		mAvailableEntities.push(entity);
		--mLivingEntityCount;
	}

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
		return mLivingLightEntityCount;
	}

	void SetSelectedEntity(std::uint32_t entity) 
	{
		this->selectedEntity = entity;
	}

	std::uint32_t GetSelectedEntity() {
		return this->selectedEntity;
	}

private:
	std::queue<Entity> mAvailableEntities{};
	std::queue<Entity> mAvailableLightEntities{};
	std::array<Signature, MAX_ENTITIES> mSignatures{};
	std::uint32_t mLivingEntityCount{};
	std::uint32_t mLivingLightEntityCount{};
	std::uint32_t selectedEntity;
};
