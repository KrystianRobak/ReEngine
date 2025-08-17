#pragma once

#include "GenericComponentArray.h"
#include "ReTypes.h"
#include "ReflectionEngine.h"
#include <memory>
#include <string>
#include <unordered_map>

class ComponentManager
{
private:
    // Helper to get the specific generic array
    std::shared_ptr<GenericComponentArray> GetComponentArray(const std::string& typeName)
    {
        assert(mComponentTypes.find(typeName.c_str()) != mComponentTypes.end() && "Component not registered before use.");

        return std::static_pointer_cast<GenericComponentArray>(mComponentArrays[typeName.c_str()]);
    }

public:
    // Register a component using its reflection data
    void RegisterComponent(const Reflection::ClassInfo* classInfo)
    {
        const char* typeName = classInfo->fullName;
        assert(mComponentTypes.find(typeName) == mComponentTypes.end() && "Registering component type more than once.");

        // Store the name and assign a ComponentType (bit index)
        mComponentTypes.insert({ typeName, mNextComponentType });

        // Create a GenericComponentArray with the size from reflection
        mComponentArrays.insert({ typeName, std::make_shared<GenericComponentArray>(classInfo->size) });

        ++mNextComponentType;
    }

    // Get the ComponentType bit index from a string name
    ComponentType GetComponentType(const std::string& typeName)
    {
        assert(mComponentTypes.find(typeName.c_str()) != mComponentTypes.end() && "Component not registered before use.");
        return mComponentTypes[typeName.c_str()];
    }

    // Add component data for an entity
    void AddComponent(Entity entity, const std::string& typeName, void* componentData)
    {
        GetComponentArray(typeName)->InsertData(entity, componentData);
    }

    // Remove a component by name
    void RemoveComponent(Entity entity, const std::string& typeName)
    {
        GetComponentArray(typeName)->RemoveData(entity);
    }

    // Get a raw pointer to a component's data
    void* GetComponent(Entity entity, const std::string& typeName)
    {
        return GetComponentArray(typeName)->GetData(entity);
    }

    void EntityDestroyed(Entity entity)
    {
        for (auto const& pair : mComponentArrays)
        {
            pair.second->EntityDestroyed(entity);
        }
    }

    // Returns all registered component names and their type IDs
    std::unordered_map<const char*, ComponentType> GetComponentsTypes()
    {
        return mComponentTypes;
    }

private:
    // Maps component name (e.g., "Engine.Transform") to a ComponentType (bit index)
    std::unordered_map<const char*, ComponentType> mComponentTypes{};

    // Maps component name to its storage array
    std::unordered_map<const char*, std::shared_ptr<IComponentArray>> mComponentArrays{};

    // The next available bit index for a new component type
    ComponentType mNextComponentType{};
};