#pragma once

#include "ReScene.h"
#include "ReCamera.h"

#include "ReflectionEngine.h"
#include "Api/AssetManagerApi.h"
#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "Engine/Core/Coordinator/EntityManager.h"

#include <filesystem>
#include "json/json.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include "MaterialSystem/Material.h"
// Include component headers for explicit casting
#include "Transform.h"
#include "RigidBody.h" 
#include "StaticMesh.h"
#include "StateMachine.h" //
#include "SkeletalMeshComponent.h"

using json = nlohmann::json;

class SceneManager
{
public:
    SceneManager()
    {
        currentScene_ = new ReScene("DefaultScene", new Camera());
    }

    Entity InstantiatePrefab(std::string path, std::shared_ptr<EntityManager> manager, Editor::IEngineEditorApi* engine)
    {
        std::ifstream in(path);
        if (!in.is_open()) {
            std::cerr << "[SceneManager] Could not open prefab: " << path << "\n";
            return MAX_ENTITIES; // Return invalid ID
        }

        json prefabJson;
        in >> prefabJson;
        in.close();

        // 1. Create the new Entity
        Entity newEntity = manager->CreateEntity();

        // 2. Parse Components (Reusing the logic)
        if (prefabJson.contains("components")) {
            for (auto& compJson : prefabJson["components"]) {
                ParseComponentJson(newEntity, compJson, engine);
            }
        }

        return newEntity;
    }

    bool SaveAsPrefab(Entity entity, std::string path, std::shared_ptr<EntityManager> manager, Editor::IEngineEditorApi* engine)
    {
        Signature sig = manager->GetSignature(entity);
        if (sig.none()) return false;

        json prefabJson;
        prefabJson["name"] = std::filesystem::path(path).stem().string();
        prefabJson["components"] = json::array();

        auto components = Reflection::Registry::Instance().GetAllComponents();

        // Loop through all registered components
        for (auto compInfo : components)
        {
            ComponentType cType = engine->GetComponentType(compInfo->fullName);
            if (sig.test(cType))
            {
                json compJson;
                compJson["type"] = compInfo->fullName;
                compJson["data"] = json::object();

                void* compPtr = engine->GetComponent(entity, compInfo->fullName);
                if (compPtr) {
                    WriteStructToJson(compInfo, compPtr, compJson["data"]);
                    prefabJson["components"].push_back(compJson);
                }
            }
        }

        std::ofstream out(path);
        if (!out.is_open()) return false;
        out << prefabJson.dump(4);
        out.close();
        return true;
    }

    // ------------------------------------------------------------------------
    // LOAD SCENE
    // ------------------------------------------------------------------------
    ReScene* LoadScene(std::string name, std::shared_ptr<EntityManager> manager, Editor::IEngineEditorApi* engine, bool keepCamera = false)
    {
        std::string filename = name;
        std::ifstream in(filename);
        if (!in.is_open())
        {
            std::cerr << "[SceneManager] Could not open scene file: " << filename << "\n";
            return currentScene_;
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

        if (sceneJson.contains("materials")) {;
            for (const auto& matJson : sceneJson["materials"]) {
                int id = matJson["id"];
                std::string path = matJson["path"];
                engine->GetAssetManager()->LoadMaterial(id, path);
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
                    if (compJson["data"].contains("AssetPath")) {
                        std::string path = compJson["data"]["AssetPath"].get<std::string>();

                        if (!path.empty()) {
                            if (compType == "StaticMesh")
                            {
                                auto sm = (StaticMesh*)compPtr;
                                sm->AssetPath = path; // Ensure string is set
                                sm->MeshResource = engine->GetAssetManager()->GetMesh(path);
                            }
                            else if (compType == "SkeletalMeshComponent")
                            {
                                auto skel = (SkeletalMeshComponent*)compPtr;
                                skel->AssetPath = path; // Ensure string is set
                                skel->MeshResource = engine->GetAssetManager()->GetSkeletalMesh(path);
                            }
                            else if (compType.find("Texture") != std::string::npos)
                            {
                                engine->GetAssetManager()->GetTexture(path);
                            }
                        }
                    }
                }
            }
        }

        // Create scene. OwnsCamera = true because 'editorCamera' was created with 'new'.
        //currentScene_ = new ReScene(name, new Camera() , true);

        engine->SendEvent(Events::Application::CAMERA_CHANGED);

        return currentScene_;
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
        sceneJson["materials"] = json::array();
        sceneJson["entities"] = json::array();

        for (const auto& [id, mat] : engine->GetAssetManager()->GetMaterials()) {
            if (!mat.path.empty()) {
                sceneJson["materials"].push_back({
                    { "id", id },
                    { "path", mat.path }
                    });
            }
        }

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

    void ParseComponentJson(Entity entity, json& compJson, Editor::IEngineEditorApi* engine)
    {
        std::string compType = compJson["type"].get<std::string>();
        auto registry = Reflection::Registry::Instance();

        // Find Component Class Info
        const Reflection::ClassInfo* compInfo = registry.FindComponent(compType);
        if (!compInfo) return;

        // Add Component to ECS
        engine->AddComponent(entity, compType);
        engine->MarkEntityDirty(entity, compType);
        void* compPtr = engine->GetComponentForWrite(entity, compType);

        // Fill Data
        if (compPtr && compJson.contains("data"))
        {
            FillStructFromJson(compInfo, compPtr, compJson["data"]);

            // --- Post-Load Logic (Moved from LoadScene) ---
            if (compType == "StateMachine")
            {
                auto sm = static_cast<StateMachine*>(compPtr);
                if (!sm->GraphAssetPath.empty()) {
                    sm->GraphResource = engine->GetAssetManager()->GetAnimationGraph(sm->GraphAssetPath);
                    // ... (Include your existing StateMachine init logic here)
                }
            }
            if (compJson["data"].contains("AssetPath")) {
                std::string path = compJson["data"]["AssetPath"].get<std::string>();
                if (!path.empty()) {
                    if (compType == "StaticMesh") {
                        auto sm = (StaticMesh*)compPtr;
                        sm->AssetPath = path;
                        sm->MeshResource = engine->GetAssetManager()->GetMesh(path);
                    }
                    else if (compType == "SkeletalMeshComponent") {
                        auto skel = (SkeletalMeshComponent*)compPtr;
                        skel->AssetPath = path;
                        skel->MeshResource = engine->GetAssetManager()->GetSkeletalMesh(path);
                    }
                    else if (compType.find("Texture") != std::string::npos) {
                        engine->GetAssetManager()->GetTexture(path);
                    }
                }
            }
        }
    }
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

            // Skip Runtime Resources
            if (tname.find("MeshResource") != std::string::npos ||
                tname.find("TextureResource") != std::string::npos ||
                tname.find("AnimationGraphResource") != std::string::npos) continue;

            // --- 2. GLM Types (The Fix for Transform) ---
            // Using .find() allows matching "struct glm::vec3" or "glm::vec3"
            if (tname.find("vec<3") != std::string::npos) {
                glm::vec3 v = *(glm::vec3*)fieldPtr; outJson[var.name] = { v.x, v.y, v.z };
            }
            else if (tname.find("vec<4") != std::string::npos) {
                glm::vec4 v = *(glm::vec4*)fieldPtr; outJson[var.name] = { v.x, v.y, v.z, v.w };
            }
            else if (tname.find("quat") != std::string::npos) {
                glm::quat q = *(glm::quat*)fieldPtr; outJson[var.name] = { q.w, q.x, q.y, q.z };
            }


            // --- 1. Basic Types ---
            else if (tname.find("float") != std::string::npos) outJson[var.name] = *(float*)fieldPtr;
            else if (tname.find("bool") != std::string::npos) outJson[var.name] = *(bool*)fieldPtr;
            // Use .find() for int to be safe against "const int", "signed int", etc.
            else if (tname.find("int") != std::string::npos) outJson[var.name] = *(int*)fieldPtr;
            else if (tname.find("string") != std::string::npos) outJson[var.name] = *(std::string*)fieldPtr;

            
            // --- 3. Blackboard (State Machine) ---
            else if (var.name == "Blackboard")
            {
                // ... (Keep existing Blackboard logic) ...
                auto& bb = *(std::unordered_map<std::string, AnimVar>*)fieldPtr;
                json bbJson = json::object();
                for (auto& [key, animVar] : bb) {
                    json varJson;
                    varJson["type"] = (int)animVar.Type;
                    if (animVar.Type == AnimVarType::Float) varJson["value"] = animVar.fVal;
                    else if (animVar.Type == AnimVarType::Int) varJson["value"] = animVar.iVal;
                    else varJson["value"] = animVar.bVal;
                    bbJson[key] = varJson;
                }
                outJson[var.name] = bbJson;
            }

            // --- 4. Recursion ---
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
            else if (tname == "glm::vec3" || tname == "vec3" || tname == "glm::vec<3, float>") {
                if (val.is_array() && val.size() == 3) *(glm::vec3*)fieldPtr = glm::vec3(val[0], val[1], val[2]);
            }
            else if (tname == "glm::vec4" || tname == "vec4" || tname == "glm::vec<4, float>") {
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