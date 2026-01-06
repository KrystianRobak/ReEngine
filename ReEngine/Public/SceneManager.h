#pragma once

#include "ReScene.h"
#include "ReCamera.h"

#include "ReflectionEngine.h"
#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "Engine/Core/Coordinator/EntityManager.h"

#include <filesystem>
#include "json/json.hpp"
#include <fstream>
#include <iostream>
#include <string>

// Include component headers for explicit casting
#include "Transform.h"
#include "RigidBody.h" 
#include "StaticMesh.h"
#include "StateMachine.h" //

using json = nlohmann::json;

class SceneManager
{
public:
    SceneManager()
    {
        currentScene_ = new ReScene("DefaultScene", new Camera());
    }

    // ------------------------------------------------------------------------
    // LOAD SCENE
    // ------------------------------------------------------------------------
    ReScene LoadScene(std::string name, std::shared_ptr<EntityManager> manager, Editor::IEngineEditorApi* engine)
    {
        std::string filename = name;
        std::ifstream in(filename);
        if (!in.is_open())
        {
            std::cerr << "[SceneManager] Could not open scene file: " << filename << "\n";
            return ReScene(name, new Camera());
        }

        json sceneJson;
        in >> sceneJson;
        in.close();

        // --- Asset Loading ---
        if (sceneJson.contains("assets")) {
            for (const auto& assetJson : sceneJson["assets"]) {
                std::string path = assetJson.get<std::string>();
                if (path.find(".reskel") != std::string::npos) engine->GetAssetManager()->GetSkeletalMesh(path);
                else engine->GetAssetManager()->GetMesh(path);
            }
        }
        if (sceneJson.contains("textures")) {
            for (const auto& texJson : sceneJson["textures"]) {
                std::string path = texJson.get<std::string>();
                engine->GetAssetManager()->GetTexture(path);
            }
        }

        // --- Entity Creation ---
        auto registry = Reflection::Registry::Instance();
        auto components = registry.GetAllComponents();
        std::unordered_map<std::string, const Reflection::ClassInfo*> componentMap;
        for (auto c : components) componentMap[c->fullName] = c;

        for (auto& entityJson : sceneJson["entities"])
        {
            Entity entity = manager->CreateEntity();

            for (auto& compJson : entityJson["components"])
            {
                std::string compType = compJson["type"].get<std::string>();
                const Reflection::ClassInfo* compInfo = componentMap.count(compType) ? componentMap[compType] : registry.FindComponent(compType);

                if (!compInfo) continue;

                engine->AddComponent(entity, compType);
                engine->MarkEntityDirty(entity, compType);
                void* compPtr = engine->GetComponentForWrite(entity, compType);

                if (compPtr && compJson.contains("data"))
                {
                    FillStructFromJson(compInfo, compPtr, compJson["data"]);

                    // --- Post-Load Logic for StateMachine ---
                    if (compType == "StateMachine")
                    {
                        auto sm = static_cast<StateMachine*>(compPtr);

                        // 1. Load the Resource (Nodes/Transitions) from the path
                        if (!sm->GraphAssetPath.empty())
                        {
                            sm->GraphResource = engine->GetAssetManager()->GetAnimationGraph(sm->GraphAssetPath);

                            // 2. Initialize defaults from resource
                            if (sm->GraphResource) {
                                // Only fill keys that weren't loaded from the scene JSON (Runtime Overrides)
                                for (auto& [key, val] : sm->GraphResource->DefaultBlackboard) {
                                    if (sm->Blackboard.find(key) == sm->Blackboard.end()) {
                                        sm->Blackboard[key] = val;
                                    }
                                }

                                // If NodeID was -1 (default) or invalid, set to entry
                                bool validNode = false;
                                for (auto& n : sm->GraphResource->Nodes) { if (n.ID == sm->CurrentNodeID) validNode = true; }

                                if (!validNode) sm->CurrentNodeID = sm->GraphResource->EntryNodeID;
                            }
                        }
                    }

                    // Asset Path fixup for Meshes
                    if (compJson["data"].contains("AssetPath")) {
                        std::string path = compJson["data"]["AssetPath"].get<std::string>();
                        if (!path.empty()) {
                            if (path.find(".reskel") != std::string::npos) engine->GetAssetManager()->GetSkeletalMesh(path);
                            else if (path.find(".retex") != std::string::npos) engine->GetAssetManager()->GetTexture(path);
                            else {
                                auto sm = (StaticMesh*)engine->GetComponent(entity, "StaticMesh");
                                sm->MeshResource = engine->GetAssetManager()->GetMesh(path);
                                sm->AssetPath = path;
                            }
                        }
                    }
                }
            }
        }

        ReScene newScene(name, new Camera());
        if (currentScene_) delete currentScene_;
        currentScene_ = new ReScene(newScene);
        return newScene;
    }

    // ------------------------------------------------------------------------
    // SAVE SCENE
    // ------------------------------------------------------------------------
    bool SerializeScene(std::string name, std::shared_ptr<EntityManager> manager, Editor::IEngineEditorApi* engine)
    {
        json sceneJson;
        sceneJson["scene_name"] = name;
        sceneJson["assets"] = engine->GetAssetManager()->GetCachedPaths();
        sceneJson["textures"] = engine->GetAssetManager()->GetCachedTexturesPaths();
        sceneJson["entities"] = json::array();

        auto components = Reflection::Registry::Instance().GetAllComponents();
        std::uint32_t entityCount = manager->GetEntityCount();

        for (std::uint32_t i = 0; i < entityCount; ++i)
        {
            Signature sig = manager->GetSignature(i);
            if (sig.none()) continue;

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
                    if (!compPtr) continue;

                    WriteStructToJson(compInfo, compPtr, compJson["data"]);
                    entityJson["components"].push_back(compJson);
                }
            }
            if (!entityJson["components"].empty())
                sceneJson["entities"].push_back(entityJson);
        }

        std::string filename = name;
        std::filesystem::path filepath(filename);
        if (filepath.has_parent_path()) {
            std::filesystem::create_directories(filepath.parent_path());
        }

        std::ofstream out(filename);
        if (!out.is_open()) return false;

        out << sceneJson.dump(4);
        out.close();

        return true;
    }

    void EntityDestroyed(Entity entity) {}
    ReScene* currentScene_;

private:
    // ------------------------------------------------------------------------
    // WriteStructToJson
    // ------------------------------------------------------------------------
    static void WriteStructToJson(const Reflection::ClassInfo* classInfo, void* basePtr, json& outJson)
    {
        for (const auto& var : classInfo->variables)
        {
            const Reflection::TypeInfo* t = var.type;
            void* fieldPtr = reinterpret_cast<char*>(basePtr) + var.offset;
            std::string tname = t->name ? t->name : "";

            // Skip pointers to resources (Runtime only)
            if (tname.find("StaticMeshData") != std::string::npos ||
                tname.find("MeshResource") != std::string::npos ||
                tname.find("TextureResource") != std::string::npos ||
                tname.find("AnimationGraphResource") != std::string::npos) continue; //

            // --- 1. Basic Types ---
            if (tname == "float") outJson[var.name] = *(float*)fieldPtr;
            else if (tname == "double") outJson[var.name] = *(double*)fieldPtr;
            else if (tname == "int" || tname == "int32_t") outJson[var.name] = *(int*)fieldPtr;
            else if (tname == "unsigned int" || tname == "uint32_t") outJson[var.name] = *(unsigned int*)fieldPtr;
            else if (tname == "bool") outJson[var.name] = *(bool*)fieldPtr;
            else if (tname == "std::string" || tname == "string" || tname == "std::basic_string<char>") outJson[var.name] = *(std::string*)fieldPtr;

            // --- 2. GLM Types ---
            else if (tname == "glm::vec3" || tname == "vec3") {
                glm::vec3 v = *(glm::vec3*)fieldPtr; outJson[var.name] = { v.x, v.y, v.z };
            }
            else if (tname == "glm::vec4" || tname == "vec4") {
                glm::vec4 v = *(glm::vec4*)fieldPtr; outJson[var.name] = { v.x, v.y, v.z, v.w };
            }
            else if (tname == "glm::quat" || tname == "quat") {
                glm::quat q = *(glm::quat*)fieldPtr; outJson[var.name] = { q.w, q.x, q.y, q.z };
            }

            // --- 3. SPECIAL: StateMachine Blackboard ---
            // The reflection engine likely returns a complex mangled name for the map, 
            // so we check the variable name directly for safety.
            else if (var.name == "Blackboard")
            {
                auto& bb = *(std::unordered_map<std::string, AnimVar>*)fieldPtr;
                json bbJson = json::object();
                for (auto& [key, animVar] : bb) {
                    json varJson;
                    varJson["type"] = (int)animVar.Type; // Save Type Enum

                    // Save Value based on Type
                    if (animVar.Type == AnimVarType::Float) varJson["value"] = animVar.fVal;
                    else if (animVar.Type == AnimVarType::Int) varJson["value"] = animVar.iVal;
                    else varJson["value"] = animVar.bVal; // Bool and Trigger

                    bbJson[key] = varJson;
                }
                outJson[var.name] = bbJson;
            }

            // --- 4. Recursive Reflected Structs ---
            else {
                const Reflection::ClassInfo* subClass = Reflection::Registry::Instance().FindClass(tname);
                if (subClass) {
                    json subJson = json::object();
                    WriteStructToJson(subClass, fieldPtr, subJson);
                    outJson[var.name] = subJson;
                }
            }
        }
    }

    // ------------------------------------------------------------------------
    // FillStructFromJson
    // ------------------------------------------------------------------------
    static void FillStructFromJson(const Reflection::ClassInfo* classInfo, void* basePtr, const json& inJson)
    {
        for (const auto& var : classInfo->variables)
        {
            const Reflection::TypeInfo* t = var.type;
            void* fieldPtr = reinterpret_cast<char*>(basePtr) + var.offset;
            std::string tname = t->name ? t->name : "";

            if (!inJson.contains(var.name)) continue;
            const json& val = inJson.at(var.name);

            // --- 1. Basic Types ---
            if (tname == "float") *(float*)fieldPtr = val.get<float>();
            else if (tname == "int" || tname == "int32_t") *(int*)fieldPtr = val.get<int>();
            else if (tname == "unsigned int") *(unsigned int*)fieldPtr = val.get<unsigned int>();
            else if (tname == "bool") *(bool*)fieldPtr = val.get<bool>();
            else if (tname == "std::string" || tname == "string") *(std::string*)fieldPtr = val.get<std::string>();

            // --- 2. GLM Types ---
            else if (tname == "glm::vec3" || tname == "vec3") {
                if (val.is_array() && val.size() == 3) *(glm::vec3*)fieldPtr = glm::vec3(val[0], val[1], val[2]);
            }
            else if (tname == "glm::vec4" || tname == "vec4") {
                if (val.is_array() && val.size() == 4) *(glm::vec4*)fieldPtr = glm::vec4(val[0], val[1], val[2], val[3]);
            }
            else if (tname == "glm::quat" || tname == "quat") {
                if (val.is_array() && val.size() == 4) *(glm::quat*)fieldPtr = glm::quat(val[0], val[1], val[2], val[3]);
            }

            // --- 3. SPECIAL: StateMachine Blackboard ---
            else if (var.name == "Blackboard")
            {
                auto& bb = *(std::unordered_map<std::string, AnimVar>*)fieldPtr;
                bb.clear(); // Clear existing to overwrite with saved data

                if (val.is_object()) {
                    for (auto& [key, varJson] : val.items()) {
                        int type = varJson["type"];

                        if (type == (int)AnimVarType::Float) {
                            bb[key] = AnimVar(varJson["value"].get<float>());
                        }
                        else if (type == (int)AnimVarType::Int) {
                            bb[key] = AnimVar(varJson["value"].get<int>());
                        }
                        else {
                            // Bool or Trigger
                            bb[key] = AnimVar(varJson["value"].get<bool>());
                            bb[key].Type = (AnimVarType)type; // Force type if it was Trigger
                        }
                    }
                }
            }

            // --- 4. Recursive Reflected Structs ---
            else {
                const Reflection::ClassInfo* subClass = Reflection::Registry::Instance().FindClass(tname);
                if (subClass && val.is_object()) FillStructFromJson(subClass, fieldPtr, val);
            }
        }
    }
};