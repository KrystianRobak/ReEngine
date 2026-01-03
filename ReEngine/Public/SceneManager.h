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

using json = nlohmann::json;

class SceneManager
{
public:
    SceneManager()
    {
        currentScene_ = new ReScene("DefaultScene", new Camera());
    }

    // ------------------------------------------------------------------------
    // LOAD SCENE (With Debug Prints)
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

        // --- Asset Loading (Unchanged) ---
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

                    // Asset Path fixup (Unchanged)
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

        // ------------------------------------------------------------------------
        // DEBUG: Explicit Casting & Printing AFTER Load
        // ------------------------------------------------------------------------
        std::cout << "\n--------------------------------------------------\n";
        std::cout << "[DEBUG] Post-Load Verification (Explicit Casting)\n";
        std::cout << "--------------------------------------------------\n";

        for (uint32_t i = 0; i < manager->GetEntityCount(); ++i)
        {
            if (!manager->GetSignature(i).any()) continue;

            // Debug Transform
            if (engine->HasComponent(i, "Transform"))
            {
                // Explicit Cast
                void* rawPtr = engine->GetComponent(i, "Transform");
                Transform* t = static_cast<Transform*>(rawPtr);

                std::cout << "Entity [" << i << "] Transform:\n";
                std::cout << "  Position: " << t->position.x << ", " << t->position.y << ", " << t->position.z << "\n";
                std::cout << "  Rotation: " << t->rotation.w << ", " << t->rotation.x << ", " << t->rotation.y << ", " << t->rotation.z << "\n";
                std::cout << "  Scale   : " << t->scale.x << ", " << t->scale.y << ", " << t->scale.z << "\n";
            }

            // Debug RigidBody
            if (engine->HasComponent(i, "RigidBody"))
            {
                // Explicit Cast
                void* rawPtr = engine->GetComponent(i, "RigidBody");
                RigidBody* rb = static_cast<RigidBody*>(rawPtr);

                std::cout << "Entity [" << i << "] RigidBody:\n";
                std::cout << "  Mass: " << rb->mass << " | IsStatic: " << rb->isStatic << "\n";
                std::cout << "  Velocity: " << rb->velocity.x << ", " << rb->velocity.y << ", " << rb->velocity.z << "\n";
            }
        }
        std::cout << "--------------------------------------------------\n\n";

        ReScene newScene(name, new Camera());
        if (currentScene_) delete currentScene_;
        currentScene_ = new ReScene(newScene);
        return newScene;
    }

    // ------------------------------------------------------------------------
    // SAVE SCENE (With Debug Prints)
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
        if (!out.is_open())
        {
            std::cerr << "[SceneManager] Failed to open " << filename << " for writing.\n";
            return false;
        }

        out << sceneJson.dump(4);
        out.close();

        std::cout << "[SceneManager] Scene serialized to " << std::filesystem::absolute(filename) << "\n";

        // ------------------------------------------------------------------------
        // DEBUG: Explicit Casting & Printing AFTER Serialization
        // ------------------------------------------------------------------------
        std::cout << "\n--------------------------------------------------\n";
        std::cout << "[DEBUG] Post-Serialization Verification (Explicit Casting)\n";
        std::cout << "--------------------------------------------------\n";

        for (uint32_t i = 0; i < entityCount; ++i)
        {
            Signature sig = manager->GetSignature(i);
            if (sig.none()) continue;

            // Debug Transform
            if (engine->HasComponent(i, "Transform"))
            {
                // Explicit Cast
                void* rawPtr = engine->GetComponent(i, "Transform");
                Transform* t = static_cast<Transform*>(rawPtr);

                std::cout << "Entity [" << i << "] Transform:\n";
                std::cout << "  Position: " << t->position.x << ", " << t->position.y << ", " << t->position.z << "\n";
                std::cout << "  Rotation: " << t->rotation.w << ", " << t->rotation.x << ", " << t->rotation.y << ", " << t->rotation.z << "\n";
                std::cout << "  Scale   : " << t->scale.x << ", " << t->scale.y << ", " << t->scale.z << "\n";
            }

            // Debug RigidBody
            if (engine->HasComponent(i, "RigidBody"))
            {
                // Explicit Cast
                void* rawPtr = engine->GetComponent(i, "RigidBody");
                RigidBody* rb = static_cast<RigidBody*>(rawPtr);

                std::cout << "Entity [" << i << "] RigidBody:\n";
                std::cout << "  Mass: " << rb->mass << " | Friction: " << rb->friction << "\n";
                std::cout << "  Velocity: " << rb->velocity.x << ", " << rb->velocity.y << ", " << rb->velocity.z << "\n";
            }
        }
        std::cout << "--------------------------------------------------\n\n";

        return true;
    }

    void EntityDestroyed(Entity entity) {}
    ReScene* currentScene_;

private:
    // ------------------------------------------------------------------------
    // FIX: WriteStructToJson (Correctly handles Quaternions)
    // ------------------------------------------------------------------------
    static void WriteStructToJson(const Reflection::ClassInfo* classInfo, void* basePtr, json& outJson)
    {
        for (const auto& var : classInfo->variables)
        {
            const Reflection::TypeInfo* t = var.type;
            void* fieldPtr = reinterpret_cast<char*>(basePtr) + var.offset;
            std::string tname = t->name ? t->name : "";

            if (tname.find("StaticMeshData") != std::string::npos ||
                tname.find("MeshResource") != std::string::npos ||
                tname.find("TextureResource") != std::string::npos) continue;

            if (tname == "float") outJson[var.name] = *(float*)fieldPtr;
            else if (tname == "double") outJson[var.name] = *(double*)fieldPtr;
            else if (tname == "int" || tname == "int32_t") outJson[var.name] = *(int*)fieldPtr;
            else if (tname == "unsigned int" || tname == "uint32_t") outJson[var.name] = *(unsigned int*)fieldPtr;
            else if (tname == "bool") outJson[var.name] = *(bool*)fieldPtr;
            else if (tname == "std::string" || tname == "string" || tname == "std::basic_string<char>") outJson[var.name] = *(std::string*)fieldPtr;

            // GLM Types
            else if (tname == "glm::vec2" || tname == "vec2" || tname == "glm::vec<2, float>") {
                glm::vec2 v = *(glm::vec2*)fieldPtr;
                outJson[var.name] = { v.x, v.y };
            }
            else if (tname == "glm::vec3" || tname == "vec3" || tname == "glm::vec<3, float>") {
                glm::vec3 v = *(glm::vec3*)fieldPtr;
                outJson[var.name] = { v.x, v.y, v.z };
            }
            else if (tname == "glm::vec4" || tname == "vec4" || tname == "glm::vec<4, float>") {
                glm::vec4 v = *(glm::vec4*)fieldPtr;
                outJson[var.name] = { v.x, v.y, v.z, v.w };
            }
            // *** FIXED QUAT HANDLING ***
            else if (tname == "glm::quat" || tname == "quat" || tname == "glm::qua<float>") {
                glm::quat q = *(glm::quat*)fieldPtr;
                // Serialize as [w, x, y, z] to match constructor order usually
                outJson[var.name] = { q.w, q.x, q.y, q.z };
            }
            else if (tname == "glm::mat4" || tname == "mat4") {
                glm::mat4 m = *(glm::mat4*)fieldPtr;
                outJson[var.name] = json::array();
                for (int i = 0; i < 4; ++i) outJson[var.name].push_back({ m[i][0], m[i][1], m[i][2], m[i][3] });
            }
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
    // FIX: FillStructFromJson (Correctly handles Quaternions)
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

            if (tname == "float") *(float*)fieldPtr = val.get<float>();
            else if (tname == "double") *(double*)fieldPtr = val.get<double>();
            else if (tname == "int" || tname == "int32_t") *(int*)fieldPtr = val.get<int>();
            else if (tname == "unsigned int" || tname == "uint32_t") *(unsigned int*)fieldPtr = val.get<unsigned int>();
            else if (tname == "bool") *(bool*)fieldPtr = val.get<bool>();
            else if (tname == "std::string" || tname == "string") *(std::string*)fieldPtr = val.get<std::string>();

            // GLM Types
            else if (tname == "glm::vec2" || tname == "vec2" || tname == "glm::vec<2, float>") {
                if (val.is_array() && val.size() == 2) *(glm::vec2*)fieldPtr = glm::vec2(val[0], val[1]);
            }
            else if (tname == "glm::vec3" || tname == "vec3" || tname == "glm::vec<3, float>") {
                if (val.is_array() && val.size() == 3) *(glm::vec3*)fieldPtr = glm::vec3(val[0], val[1], val[2]);
            }
            else if (tname == "glm::vec4" || tname == "vec4" || tname == "glm::vec<4, float>") {
                if (val.is_array() && val.size() == 4) *(glm::vec4*)fieldPtr = glm::vec4(val[0], val[1], val[2], val[3]);
            }
            // *** FIXED QUAT HANDLING ***
            else if (tname == "glm::quat" || tname == "quat" || tname == "glm::qua<float>") {
                if (val.is_array() && val.size() == 4) {
                    // JSON was saved as {w, x, y, z}
                    *(glm::quat*)fieldPtr = glm::quat(val[0], val[1], val[2], val[3]);
                }
            }
            else if (tname == "glm::mat4" || tname == "mat4") {
                glm::mat4 m(1.0f);
                if (val.is_array() && val.size() == 4) {
                    for (int i = 0; i < 4; ++i)
                        for (int j = 0; j < 4; ++j) m[i][j] = val[i][j];
                }
                *(glm::mat4*)fieldPtr = m;
            }
            else {
                const Reflection::ClassInfo* subClass = Reflection::Registry::Instance().FindClass(tname);
                if (subClass && val.is_object()) FillStructFromJson(subClass, fieldPtr, val);
            }
        }
    }
};