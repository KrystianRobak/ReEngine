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
    // Destructor to clean up raw pointers
    ~SystemManager()
    {
        for (auto const& pair : mSystems)
        {
            delete pair.second;
        }
        mSystems.clear();
    }

    // Register a system using its reflection data
    System* RegisterSystem(const Reflection::ClassInfo* classInfo)
    {
        const char* typeName = classInfo->fullName;
        assert(mSystems.find(typeName) == mSystems.end() && "Registering system more than once.");
        assert(classInfo->construct && "Reflected class has no default constructor.");

        // Use the reflection 'construct' function to create a new instance
        void* newInstanceRaw = classInfo->construct();

        // Cast the void* to the base System*
        System* newSystem = static_cast<System*>(newInstanceRaw);

        // Store the raw pointer in the map
        mSystems.emplace(typeName, newSystem);

        return newSystem;
    }

    // Get a system by its string name
    System* GetSystem(const std::string& typeName)
    {
        auto it = mSystems.find(typeName);
        if (it != mSystems.end()) {
            return it->second; // Return the raw System*
        }
        return nullptr;
    }

    // Set a system's component signature using its string name
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

            // Ensure the signature exists before accessing it
            if (mSignatures.count(type)) {
                auto const& systemSignature = mSignatures[type];

                // If the entity's signature contains all the bits from the system's signature
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
    // Maps system name to its required component signature
    std::unordered_map<std::string, Signature> mSignatures{};

    // Maps system name to the actual system instance (raw pointer)
    std::unordered_map<std::string, System*> mSystems{};
};