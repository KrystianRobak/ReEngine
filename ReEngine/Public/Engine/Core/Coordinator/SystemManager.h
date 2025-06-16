#pragma once

#include "Engine/Systems/System.h"
#include "Engine/Core/Types.h"
#include <cassert>
#include <memory>
#include <unordered_map>



class SystemManager
{
public:
	template<typename T>
	std::shared_ptr<T> RegisterSystem()
	{
		const char* typeName = typeid(T).name();

		assert(mSystems.find(typeName) == mSystems.end() && "Registering system more than once.");

		auto system = std::make_shared<T>();
		mSystems.emplace(typeName, system);
		return system;
	}

	template<typename T>
	std::shared_ptr<T> GetSystem()
	{
		const char* typeName = typeid(T).name();
		auto it = mSystems.find(typeName);
		if (it != mSystems.end()) {
			// Convert the shared_ptr<void> to the correct type
			return std::static_pointer_cast<T>(it->second);
		}
		else {
			return nullptr;
		}
	}

	template<typename T>
	void SetSignature(Signature signature)
	{
		const char* typeName = typeid(T).name();

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

private:
	std::unordered_map<const char*, Signature> mSignatures{};
	std::unordered_map<const char*, std::shared_ptr<System>> mSystems{};
};
