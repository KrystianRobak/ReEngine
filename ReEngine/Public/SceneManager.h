#pragma once

#include "ReScene.h"
#include "ReCamera.h"

#include "ReflectionEngine.h"
#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "Engine/Core/Coordinator/EntityManager.h"

#include "json/json.hpp"
#include <fstream>
#include <iostream>
#include <string>

using json = nlohmann::json;

class SceneManager
{
public:
	SceneManager()
	{
		currentScene_ = new ReScene("DefaultScene", new Camera());
	}

	// Load a scene from "<name>.json"
	ReScene LoadScene(std::string name, std::shared_ptr<EntityManager> manager, std::shared_ptr<Editor::IEngineEditorApi> engine)
	{
		std::string filename = "content/" + name + ".json";
		std::ifstream in(filename);
		if (!in.is_open())
		{
			std::cerr << "[SceneManager] Could not open scene file: " << filename << "\n";
			return ReScene(name, new Camera());
		}

		json sceneJson;
		in >> sceneJson;
		in.close();

		auto registry = Reflection::Registry::Instance();
		auto components = registry.GetAllComponents();
		std::unordered_map<std::string, const Reflection::ClassInfo*> componentMap;
		for (auto c : components)
			componentMap[c->fullName] = c;

		// iterate through entities (no id, order defines them)
		for (auto& entityJson : sceneJson["entities"])
		{
			Entity entity = manager->CreateEntity();

			for (auto& compJson : entityJson["components"])
			{
				std::string compType = compJson["type"].get<std::string>();
				const Reflection::ClassInfo* compInfo = nullptr;

				if (componentMap.count(compType))
					compInfo = componentMap[compType];
				else
					compInfo = registry.FindComponent(compType);

				if (!compInfo)
				{
					std::cerr << "[SceneManager] Unknown component type while loading: " << compType << "\n";
					continue;
				}

				engine->AddComponent(entity, compType);
				void* compPtr = engine->GetComponent(entity, compType);
				if (!compPtr)
				{
					std::cerr << "[SceneManager] Failed to get component pointer for " << compType << "\n";
					continue;
				}

				if (compJson.contains("data"))
					FillStructFromJson(compInfo, compPtr, compJson["data"]);
			}
		}

		std::cout << "[SceneManager] Loaded scene: " << name << " successfully.\n";
		ReScene newScene(name, new Camera());
		if (currentScene_) delete currentScene_;
		currentScene_ = new ReScene(newScene);
		return newScene;
	}

	// Save scene to "<name>.json"
	bool SerializeScene(std::string name, std::shared_ptr<EntityManager> manager, std::shared_ptr<Editor::IEngineEditorApi> engine)
	{
		json sceneJson;
		sceneJson["scene_name"] = name;
		sceneJson["entities"] = json::array();

		auto components = Reflection::Registry::Instance().GetAllComponents();
		std::uint32_t entityCount = manager->GetEntityCount();

		for (std::uint32_t i = 0; i < entityCount; ++i)
		{
			Signature sig = manager->GetSignature(i);
			json entityJson;
			entityJson["components"] = json::array();

			for (auto compInfo : components)
			{
				ComponentType cType = engine->GetComponentType(compInfo->fullName);
				if (sig.test(cType))
				{
					json compJson;
					compJson["type"] = compInfo->fullName;
					compJson["data"] = json::object();

					void* compPtr = engine->GetComponent(i, compInfo->fullName);
					if (!compPtr)
					{
						std::cerr << "[SceneManager] SerializeScene: null ptr for component "
							<< compInfo->fullName << " on entity " << i << "\n";
						continue;
					}

					WriteStructToJson(compInfo, compPtr, compJson["data"]);
					entityJson["components"].push_back(compJson);
				}
			}

			// Only include entities that have at least one component
			if (!entityJson["components"].empty())
				sceneJson["entities"].push_back(entityJson);
		}

		std::string filename = "content/" + name + ".json";
		std::ofstream out(filename);
		if (!out.is_open())
		{
			std::cerr << "[SceneManager] Failed to open " << filename << " for writing.\n";
			return false;
		}
		out << sceneJson.dump(4);
		out.close();

		std::cout << "[SceneManager] Scene serialized successfully to " << filename << "\n";
		return true;
	}

	void EntityDestroyed(Entity entity)
	{

	}

	ReScene* currentScene_;

private:
	// Recursive serializer: writes fields of a struct/class described by classInfo into outJson
	static void WriteStructToJson(const Reflection::ClassInfo* classInfo, void* basePtr, json& outJson)
	{
		for (const auto& var : classInfo->variables)
		{
			const Reflection::TypeInfo* t = var.type;
			void* fieldPtr = reinterpret_cast<char*>(basePtr) + var.offset;
			std::string tname = t->name ? t->name : "";

			// Primitive types
			if (tname == "float")
				outJson[var.name] = *(float*)fieldPtr;
			else if (tname == "double")
				outJson[var.name] = *(double*)fieldPtr;
			else if (tname == "int" || tname == "int32_t")
				outJson[var.name] = *(int*)fieldPtr;
			else if (tname == "unsigned int" || tname == "uint32_t")
				outJson[var.name] = *(unsigned int*)fieldPtr;
			else if (tname == "bool")
				outJson[var.name] = *(bool*)fieldPtr;
			else if (tname == "std::string" || tname == "string")
				outJson[var.name] = *(std::string*)fieldPtr;

			// --- GLM types ---
			else if (tname == "glm::vec2" || tname == "vec2")
			{
				glm::vec2 v = *(glm::vec2*)fieldPtr;
				outJson[var.name] = { v.x, v.y };
			}
			else if (tname == "glm::vec3" || tname == "vec3")
			{
				glm::vec3 v = *(glm::vec3*)fieldPtr;
				outJson[var.name] = { v.x, v.y, v.z };
			}
			else if (tname == "glm::vec4" || tname == "vec4")
			{
				glm::vec4 v = *(glm::vec4*)fieldPtr;
				outJson[var.name] = { v.x, v.y, v.z, v.w };
			}
			else if (tname == "glm::mat4" || tname == "mat4")
			{
				glm::mat4 m = *(glm::mat4*)fieldPtr;
				outJson[var.name] = json::array();
				for (int i = 0; i < 4; ++i)
				{
					outJson[var.name].push_back({ m[i][0], m[i][1], m[i][2], m[i][3] });
				}
			}

			// --- Nested reflected structs/classes ---
			else
			{
				const Reflection::ClassInfo* subClass = Reflection::Registry::Instance().FindClass(tname);
				if (subClass)
				{
					json subJson = json::object();
					WriteStructToJson(subClass, fieldPtr, subJson);
					outJson[var.name] = subJson;
				}
				else
				{
					std::cerr << "[SceneManager] Unknown type '" << tname << "' for variable '" << var.name << "'\n";
				}
			}
		}
	}

	// Recursive deserializer: fills fields of classInfo from inJson into basePtr
	static void FillStructFromJson(const Reflection::ClassInfo* classInfo, void* basePtr, const json& inJson)
	{
		for (const auto& var : classInfo->variables)
		{
			const Reflection::TypeInfo* t = var.type;
			void* fieldPtr = reinterpret_cast<char*>(basePtr) + var.offset;
			std::string tname = t->name ? t->name : "";

			if (!inJson.contains(var.name))
				continue;

			const json& val = inJson.at(var.name);

			// Primitive types
			if (tname == "float")
				*(float*)fieldPtr = val.get<float>();
			else if (tname == "double")
				*(double*)fieldPtr = val.get<double>();
			else if (tname == "int" || tname == "int32_t")
				*(int*)fieldPtr = val.get<int>();
			else if (tname == "unsigned int" || tname == "uint32_t")
				*(unsigned int*)fieldPtr = val.get<unsigned int>();
			else if (tname == "bool")
				*(bool*)fieldPtr = val.get<bool>();
			else if (tname == "std::string" || tname == "string")
				*(std::string*)fieldPtr = val.get<std::string>();

			// --- GLM types ---
			else if (tname == "glm::vec2" || tname == "vec2")
			{
				if (val.is_array() && val.size() == 2)
					*(glm::vec2*)fieldPtr = glm::vec2(val[0].get<float>(), val[1].get<float>());
			}
			else if (tname == "glm::vec3" || tname == "vec3")
			{
				if (val.is_array() && val.size() == 3)
					*(glm::vec3*)fieldPtr = glm::vec3(val[0].get<float>(), val[1].get<float>(), val[2].get<float>());
			}
			else if (tname == "glm::vec4" || tname == "vec4")
			{
				if (val.is_array() && val.size() == 4)
					*(glm::vec4*)fieldPtr = glm::vec4(val[0].get<float>(), val[1].get<float>(), val[2].get<float>(), val[3].get<float>());
			}
			else if (tname == "glm::mat4" || tname == "mat4")
			{
				glm::mat4 m(1.0f);
				if (val.is_array() && val.size() == 4)
				{
					for (int i = 0; i < 4; ++i)
					{
						if (val[i].is_array() && val[i].size() == 4)
						{
							for (int j = 0; j < 4; ++j)
								m[i][j] = val[i][j].get<float>();
						}
					}
				}
				*(glm::mat4*)fieldPtr = m;
			}

			// --- Nested reflected structs/classes ---
			else
			{
				const Reflection::ClassInfo* subClass = Reflection::Registry::Instance().FindClass(tname);
				if (subClass && val.is_object())
				{
					FillStructFromJson(subClass, fieldPtr, val);
				}
				else
				{
					std::cerr << "[SceneManager] Unknown type '" << tname << "' for variable '" << var.name << "'\n";
				}
			}
		}
	}
};
