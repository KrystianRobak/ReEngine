#pragma once

#include "System/System.h"
#include "Types.h"
#include "ReflectionEngine.h"
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

class SystemManager
{
public:

    ~SystemManager()
    {
        for (auto const& pair : mSystems)
        {
            delete pair.second;
        }
        mSystems.clear();
    }

    System* RegisterSystem(const Reflection::ClassInfo* classInfo)
    {
        const char* typeName = classInfo->fullName;
        if (mSystems.find(typeName) != mSystems.end()) return nullptr;

        assert(classInfo->construct && "Reflected class has no default constructor.");

        void* newInstanceRaw = classInfo->construct();

        System* newSystem = static_cast<System*>(newInstanceRaw);

        mSystems.emplace(typeName, newSystem);

        return newSystem;
    }

    void UnregisterSystem(const std::string& typeName)
    {
        auto it = mSystems.find(typeName);
        if (it != mSystems.end())
        {
            delete it->second;
            mSystems.erase(it);
        }

        mSignatures.erase(typeName);
    }

    void OnBeginSimulation()
    {
        for (auto& pair : mSystems)
        {
            pair.second->OnBeginSimulation();
        }
    }

    void OnEndSimulation()
    {
        for (auto& pair : mSystems)
        {
            pair.second->OnEndSimulation();
        }
    }


    System* GetSystem(const std::string& typeName)
    {
        auto it = mSystems.find(typeName);
        if (it != mSystems.end()) {
            return it->second;
        }
        return nullptr;
    }

    void SetSignature(const std::string& typeName, Signature signature)
    {
        assert(mSystems.find(typeName) != mSystems.end() && "System used before registered.");
        mSignatures.insert({ typeName, signature });
    }

    void EntityDestroyed(Entity entity)
    {
        for (auto const& pair : mSystems)
        {
            auto const& system = pair.second;
            system->GetEntities().erase(entity);
        }
    }

    void EntitySignatureChanged(Entity entity, Signature entitySignature)
    {
        for (auto const& pair : mSystems)
        {
            auto const& type = pair.first;
            auto const& system = pair.second;

            if (mSignatures.count(type)) {
                auto const& systemSignature = mSignatures[type];

                if ((entitySignature & systemSignature) == systemSignature)
                {
                    system->GetEntities().insert(entity);
                }
                else
                {
                    system->GetEntities().erase(entity);
                }
            }
        }
    }

private:
    std::unordered_map<std::string, Signature> mSignatures{};

    std::unordered_map<std::string, System*> mSystems{};
};