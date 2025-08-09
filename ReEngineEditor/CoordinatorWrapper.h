#pragma once

#include <iostream>

typedef uint32_t(*CoordinatorCreateEntityFunc)();
typedef uint32_t(*CoordinatorCreateLightEntityFunc)();
typedef void(*CoordinatorDestroyEntityFunc)(uint32_t);
typedef uint32_t(*CoordinatorGetEntitiesAmountFunc)();
typedef uint32_t(*CoordinatorGetLightEntitiesAmountFunc)();
typedef void(*CoordinatorSetSelectedEntityFunc)(uint32_t);
typedef uint32_t(*CoordinatorGetSelectedEntityFunc)();
typedef void (*CoordinatorGetComponentTypesFunc)();

class CoordinatorWrapper
{
public:
	CoordinatorWrapper(
		CoordinatorCreateEntityFunc createEntityFunc,
		CoordinatorCreateLightEntityFunc createLightEntityFunc,
		CoordinatorDestroyEntityFunc destroyEntityFunc,
		CoordinatorGetEntitiesAmountFunc getEntitiesAmountFunc,
		CoordinatorGetLightEntitiesAmountFunc getLightEntitiesAmountFunc,
		CoordinatorSetSelectedEntityFunc setSelectedEntityFunc,
		CoordinatorGetSelectedEntityFunc getSelectedEntityFunc,
		CoordinatorGetComponentTypesFunc getComponentTypesFunc
	)
		: CreateEntityFunc(createEntityFunc),
		CreateLightEntityFunc(createLightEntityFunc),
		DestroyEntityFunc(destroyEntityFunc),
		GetEntitiesAmountFunc(getEntitiesAmountFunc),
		GetLightEntitiesAmountFunc(getLightEntitiesAmountFunc),
		SetSelectedEntityFunc(setSelectedEntityFunc),
		GetSelectedEntityFunc(getSelectedEntityFunc),
		GetComponentTypesFunc(getComponentTypesFunc)
	{}

	// === Wrapped Calls ===
	uint32_t CreateEntity() {
		return CreateEntityFunc();
	}

	uint32_t CreateLightEntity() {
		return CreateLightEntityFunc();
	}

	void DestroyEntity(uint32_t entity) {
		DestroyEntityFunc(entity);
	}

	uint32_t GetEntitiesAmount() {
		return GetEntitiesAmountFunc();
	}

	uint32_t GetLightEntitiesAmount() {
		return GetLightEntitiesAmountFunc();
	}

	void SetSelectedEntity(uint32_t entity) {
		SetSelectedEntityFunc(entity);
	}

	uint32_t GetSelectedEntity() {
		return GetSelectedEntityFunc();
	}

	void GetComponentTypes() {
		GetComponentTypesFunc();
	}

private:
	CoordinatorCreateEntityFunc CreateEntityFunc;
	CoordinatorCreateLightEntityFunc CreateLightEntityFunc;
	CoordinatorDestroyEntityFunc DestroyEntityFunc;
	CoordinatorGetEntitiesAmountFunc GetEntitiesAmountFunc;
	CoordinatorGetLightEntitiesAmountFunc GetLightEntitiesAmountFunc;
	CoordinatorSetSelectedEntityFunc SetSelectedEntityFunc;
	CoordinatorGetSelectedEntityFunc GetSelectedEntityFunc;
	CoordinatorGetComponentTypesFunc GetComponentTypesFunc;
};

