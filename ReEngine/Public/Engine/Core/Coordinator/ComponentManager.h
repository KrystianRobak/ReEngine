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
    std::shared_ptr<GenericComponentArray> GetComponentArray(const std::string& typeName)
    {
        assert(mComponentTypes.find(typeName.c_str()) != mComponentTypes.end() && "Component not registered before use.");

        return std::static_pointer_cast<GenericComponentArray>(mComponentArrays[typeName]);
    }

public:
    void RegisterComponent(const Reflection::ClassInfo* classInfo, bool IsDoubleBuffered = false)
    {
        const char* typeName = classInfo->fullName;
        if (mComponentTypes.find(typeName) != mComponentTypes.end()) return;

        mComponentTypes.insert({ typeName, mNextComponentType });

        mComponentArrays.insert({ typeName, std::make_shared<GenericComponentArray>(classInfo->size, IsDoubleBuffered) });

        ++mNextComponentType;
    }

    void UnregisterComponent(const std::string& typeName)
    {
        mComponentTypes.erase(typeName);
        mComponentArrays.erase(typeName);
    }

    void* GetComponentForWrite(Entity entity, const std::string& typeName)
    {
        return GetComponentArray(typeName)->GetDataForWrite(entity);
    }

    void SwapComponentBuffers(const std::string& typeName)
    {
        assert(mComponentTypes.find(typeName.c_str()) != mComponentTypes.end() && "Component not registered before use.");
        GetComponentArray(typeName)->SwapData();
    }

    void MarkEntityDirty(Entity entity, const std::string& typeName)
    {
        assert(mComponentTypes.find(typeName.c_str()) != mComponentTypes.end() && "Component not registered before use.");
        GetComponentArray(typeName)->MarkDirty(entity);
	}

    ComponentType GetComponentType(const std::string& typeName)
    {
        assert(mComponentTypes.find(typeName.c_str()) != mComponentTypes.end() && "Component not registered before use.");
        return mComponentTypes[typeName.c_str()];
    }

    void AddComponent(Entity entity, const std::string& typeName, void* componentData)
    {
        GetComponentArray(typeName)->InsertData(entity, componentData);
    }

    void RemoveComponent(Entity entity, const std::string& typeName)
    {
        GetComponentArray(typeName)->RemoveData(entity);
    }

    void* GetComponent(Entity entity, const std::string& typeName)
    {
        return GetComponentArray(typeName)->GetData(entity);
    }

    std::vector<char>* GetComponentWriteBuffer(const std::string& typeName)
    {
        return GetComponentArray(typeName)->GetWriteBuffer();
    }

    std::vector<char>* GetComponentReadBuffer(const std::string& typeName)
    {
        return GetComponentArray(typeName)->GetReadBuffer();
    }

    void EntityDestroyed(Entity entity)
    {
        for (auto const& pair : mComponentArrays)
        {
            pair.second->EntityDestroyed(entity);
        }
    }

    std::unordered_map<std::string, ComponentType> GetComponentsTypes()
    {
        return mComponentTypes;
    }

private:
    std::unordered_map<std::string, ComponentType> mComponentTypes{};

    std::unordered_map<std::string, std::shared_ptr<IComponentArray>> mComponentArrays{};

    ComponentType mNextComponentType{};
};