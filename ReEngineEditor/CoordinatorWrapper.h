#pragma once

#include <windows.h>
#include "Logger.h"
#include "ReflectionEngine.h"
#include <iostream>
#include <cstdint> // Use cstdint for integer types

// --- Core Entity Management ---
typedef uint32_t(*CoordinatorCreateEntityFunc)();
typedef uint32_t(*CoordinatorCreateLightEntityFunc)();
typedef void(*CoordinatorDestroyEntityFunc)(uint32_t);
typedef uint32_t(*CoordinatorGetEntitiesAmountFunc)();
typedef uint32_t(*CoordinatorGetLightEntitiesAmountFunc)();
typedef void(*CoordinatorSetSelectedEntityFunc)(uint32_t);
typedef uint32_t(*CoordinatorGetSelectedEntityFunc)();

// --- Component Management ---
typedef void(*CoordinatorRegisterComponentFunc)(const Reflection::ClassInfo*);
typedef void(*CoordinatorAddComponentFunc)(uint32_t, const char*, void*);
typedef void(*CoordinatorRemoveComponentFunc)(uint32_t, const char*);
typedef void* (*CoordinatorGetComponentFunc)(uint32_t, const char*);
typedef bool(*CoordinatorHasComponentFunc)(uint32_t, const char*);

// --- Component Type Information ---
typedef int(*CoordinatorGetComponentTypeCountFunc)();
typedef const char* (*CoordinatorGetComponentTypeNameFunc)(int);


class CoordinatorWrapper
{
public:
	CoordinatorWrapper()
	{}

	void LoadFunctions(HMODULE engineDLL) {
		CreateEntityFunc;
		CreateLightEntityFunc;
		DestroyEntityFunc;
		GetEntitiesAmountFunc;
		GetLightEntitiesAmountFunc;
		SetSelectedEntityFunc;
		GetSelectedEntityFunc;

		RegisterComponentFunc = (CoordinatorRegisterComponentFunc)GetProcAddress(engineDLL, "Coordinator_RegisterComponent");
		AddComponentFunc = (CoordinatorAddComponentFunc)GetProcAddress(engineDLL, "Coordinator_AddComponent");
		RemoveComponentFunc = (CoordinatorRemoveComponentFunc)GetProcAddress(engineDLL, "Coordinator_RemoveComponent");
		GetComponentFunc = (CoordinatorGetComponentFunc)GetProcAddress(engineDLL, "Coordinator_GetComponent");
		HasComponentFunc = (CoordinatorHasComponentFunc)GetProcAddress(engineDLL, "Coordinator_HasComponent");

		GetComponentTypeCountFunc = (CoordinatorGetComponentTypeCountFunc)GetProcAddress(engineDLL, "Coordinator_GetComponentTypeCount");
		GetComponentTypeNameFunc = (CoordinatorGetComponentTypeNameFunc)GetProcAddress(engineDLL, "Coordinator_GetComponentTypeName");

	}

	// === Wrapped Entity Calls ===
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

	// === Wrapped Component Calls ===
	void AddComponent(uint32_t entity, const char* componentTypeName, void* componentData) {
		AddComponentFunc(entity, componentTypeName, componentData);
	}

	void RemoveComponent(uint32_t entity, const char* componentTypeName) {
		RemoveComponentFunc(entity, componentTypeName);
	}

	void* GetComponent(uint32_t entity, const char* componentTypeName) {
		return GetComponentFunc(entity, componentTypeName);
	}

	bool HasComponent(uint32_t entity, const char* componentTypeName) {
		return HasComponentFunc(entity, componentTypeName);
	}

	// === Wrapped Component Type Info Calls ===
	int GetComponentTypeCount() {
		return GetComponentTypeCountFunc();
	}

	const char* GetComponentTypeName(int index) {
		return GetComponentTypeNameFunc(index);
	}

	void RegisterComponent(const Reflection::ClassInfo* Class)
	{
		RegisterComponentFunc(Class);
	}

	bool IsProperlyLoaded() {
		if (!RegisterComponentFunc || !AddComponentFunc || !RemoveComponentFunc || !GetComponentFunc || !HasComponentFunc) {
			LOGF_ERROR("%s", "Failed while initializing coordinator functions!")
				return false;
		}
		return true;
	}


public:
	// Core Entity Management
	CoordinatorCreateEntityFunc CreateEntityFunc;
	CoordinatorCreateLightEntityFunc CreateLightEntityFunc;
	CoordinatorDestroyEntityFunc DestroyEntityFunc;
	CoordinatorGetEntitiesAmountFunc GetEntitiesAmountFunc;
	CoordinatorGetLightEntitiesAmountFunc GetLightEntitiesAmountFunc;
	CoordinatorSetSelectedEntityFunc SetSelectedEntityFunc;
	CoordinatorGetSelectedEntityFunc GetSelectedEntityFunc;

	// Component Management
	CoordinatorRegisterComponentFunc RegisterComponentFunc;
	CoordinatorAddComponentFunc AddComponentFunc;
	CoordinatorRemoveComponentFunc RemoveComponentFunc;
	CoordinatorGetComponentFunc GetComponentFunc;
	CoordinatorHasComponentFunc HasComponentFunc;

	// Component Type Information
	CoordinatorGetComponentTypeCountFunc GetComponentTypeCountFunc;
	CoordinatorGetComponentTypeNameFunc GetComponentTypeNameFunc;       
};