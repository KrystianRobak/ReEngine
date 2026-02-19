#include "PropertyPanel.h"
#include "Engine/Components/Renderable.h"
#include "Engine/Components/Gravity.h"
#include "Engine/Components/RigidBody.h"
//#include "Engine/Components/Collision.h"
#include "Engine/Components/Player.h"
#include "Engine/Components/Animated.h"
#include "Engine/Components/LightSource.h"
#include <Engine/Components/BehaviourScript.h>
#include <filesystem>

#include "StaticMesh.h"
#include "SkeletalMeshComponent.h"
#include "StateMachine.h"
#include <gtc/quaternion.hpp>
#include <gtc/type_ptr.hpp>
#include <MeshCollider.h>

inline std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;

}

// Helper to safely edit Quaternions as Euler Angles (Degrees)
static bool DrawQuatAsEuler(const char* label, glm::quat& q)
{
    glm::vec3 euler = glm::degrees(glm::eulerAngles(q));

    // Check if the user changed the values
    if (ImGui::DragFloat3(label, glm::value_ptr(euler), 0.1f))
    {
        // Convert back to quaternion
        q = glm::quat(glm::radians(euler));
        return true;
    }
    return false;
}

// Helper to find ClassInfo by string name (Naive implementation)
// You might want to move this to your Reflection::Registry class
const Reflection::ClassInfo* FindReflectedType(const char* typeName)
{
    auto allComponents = Reflection::Registry::Instance().GetAllComponents();
    for (auto info : allComponents)
    {
        // Simple string match. In a real engine, use TypeIDs or HashMaps.
        if (strcmp(info->name, typeName) == 0)
            return info;
    }
    return nullptr;
}

void PropertyPanel::RenderVariable(const char* varName, const char* typeName, void* varPtr, void* writeData, const char* name)
{
    ImGui::PushID(varPtr); // Ensure ImGui IDs are unique per variable memory address

    // 1. --- Primitives ---
    if (strcmp(typeName, "float") == 0) {
        ImGui::DragFloat(varName, (float*)varPtr, 0.1f);
    }
    else if (strcmp(typeName, "int") == 0) {
        ImGui::InputInt(varName, (int*)varPtr);
    }
    else if (strcmp(typeName, "bool") == 0) {
        ImGui::Checkbox(varName, (bool*)varPtr);
    }
    else if (strcmp(typeName, "glm::vec<3, float>") == 0 || strcmp(typeName, "glm::vec3") == 0) {
        ImGui::DragFloat3(varName, (float*)varPtr, 0.1f);
    }
    else if (strcmp(typeName, "glm::vec<4, float>") == 0 || strcmp(typeName, "glm::vec4") == 0) {
        ImGui::DragFloat4(varName, (float*)varPtr, 0.1f);
    }
    else if (strcmp(typeName, "glm::qua<float>") == 0 || strcmp(typeName, "glm::quat") == 0) {
        DrawQuatAsEuler(varName, *(glm::quat*)varPtr);
    }
    else if (strcmp(typeName, "std::string") == 0 || strcmp(typeName, "std::basic_string<char>") == 0) {
        std::string* strPtr = (std::string*)varPtr;

        // 1. Static Mesh Asset
        if (strcmp(name, "StaticMesh") == 0 && strcmp(varName, "AssetPath") == 0) {
            if (DrawAssetSlot(varName, *strPtr, "ASSET_STATIC_MESH", FileType::StaticMesh)) {
                // FORCE UPDATE: Clear the resource handle so the System re-fetches it next frame
                ((StaticMesh*)writeData)->MeshResource = engineAPI->GetAssetManager()->GetMesh(*strPtr);
                //engineAPI->MarkEntityDirty(entity, componentInfo->name);
            }
        }
        // 2. Skeletal Mesh Asset
        else if (strcmp(name, "SkeletalMeshComponent") == 0 && strcmp(varName, "AssetPath") == 0) {
            if (DrawAssetSlot(varName, *strPtr, "ASSET_SKELETAL_MESH", FileType::SkeletalMesh)) {
                // FORCE UPDATE
                ((SkeletalMeshComponent*)writeData)->MeshResource = engineAPI->GetAssetManager()->GetMesh(*strPtr);
                //engineAPI->MarkEntityDirty(entity, componentInfo->name);
            }
        }
        // 3. Animation Graph (State Machine)
        else if (strcmp(name, "StateMachine") == 0 && strcmp(varName, "GraphAssetPath") == 0) {
            if (DrawAssetSlot(varName, *strPtr, "ASSET_ANIMATION", FileType::Animation)) {
                // FORCE UPDATE
                ((StateMachine*)writeData)->GraphResource = engineAPI->GetAssetManager()->GetAnimationGraph(*strPtr);
                //engineAPI->MarkEntityDirty(entity, componentInfo->name);
            }
        }
        // 4. Fallback for generic strings
        else {
            static char buf[256];
            strncpy_s(buf, strPtr->c_str(), 256);
            if (ImGui::InputText(varName, buf, 256)) {
                *strPtr = std::string(buf);
            }
        }
    }
    else
    {
        // Check if this unknown type is actually another Reflected Component/Struct
        const Reflection::ClassInfo* nestedType = FindReflectedType(typeName);

        if (nestedType)
        {
            // Create a tree node for the nested struct (e.g., "CameraTransform")
            if (ImGui::TreeNode(varName))
            {
                // Iterate through the nested struct's variables
                for (const auto& childVar : nestedType->variables)
                {
                    // Calculate pointer to the nested member
                    // Parent Pointer + Offset = Child Pointer
                    void* childPtr = (char*)varPtr + childVar.offset;

                    // RECURSE!
                    RenderVariable(childVar.name, childVar.type->name, childPtr, writeData, name);
                }
                ImGui::TreePop();
            }
        }
        else
        {
            ImGui::TextDisabled("%s (%s) - [Not Supported]", varName, typeName);
        }
    }

    ImGui::PopID();
}

void RenderComponentsMenu(std::int32_t& entity, std::bitset<32>& signature) 
{

}

void PropertyPanel::OnInit()
{

}

void PropertyPanel::ForEachComponent(const char* header, std::vector<const Reflection::ClassInfo*> Components, std::function<void(Entity, const char*)> function, bool IsAdding)
{
    if (ImGui::CollapsingHeader(header))
    {
        for (auto component : Components)
        {
            Entity entity = engineAPI->GetSelectedEntity();

            if (IsAdding)
            {
                if (!engineAPI->HasComponent(entity, component->name))
                {
                    if (ImGui::Button(component->name))
                    {
                        function(entity, component->name);
                    }
                }
            }
            else
            {
                if (engineAPI->HasComponent(entity, component->name))
                {
                    if (ImGui::Button(component->name))
                    {
                        function(entity, component->name);
                    }
                }
            }
            
        }
    }
}


void PropertyPanel::Render()
{
	ImGui::Begin("Properties");

    auto Components = Reflection::Registry::Instance().GetAllComponents();
    Entity entity = engineAPI->GetSelectedEntity();

    if (entity != 111)
    {
        ForEachComponent("Add component", Components,
            [this](Entity entity, const char* name) {
                Signature signature = engineAPI->GetEntitySignature(entity);
                if (!engineAPI->HasComponent(entity, name))
                {
                    engineAPI->AddComponent(entity, name);

                    std::string compName = name;

                    if (compName == "MeshCollider")
                    {
                        if (engineAPI->HasComponent(entity, "StaticMesh"))
                        {
                            auto* meshCol = (MeshCollider*)engineAPI->GetComponentForWrite(entity, "MeshCollider");
                            auto* staticMesh = (StaticMesh*)engineAPI->GetComponent(entity, "StaticMesh");

                            if (meshCol && staticMesh && staticMesh->MeshResource && staticMesh->MeshResource->cpuMesh)
                            {
                                meshCol->meshData = staticMesh->MeshResource->cpuMesh;
                                std::cout << "[Editor] Auto-linked StaticMesh to new MeshCollider.\n";
                            }
                        }
                    }
                    else if (compName == "StaticMesh")
                    {
                        if (engineAPI->HasComponent(entity, "MeshCollider"))
                        {
                            auto* meshCol = (MeshCollider*)engineAPI->GetComponentForWrite(entity, "MeshCollider");
                            auto* staticMesh = (StaticMesh*)engineAPI->GetComponent(entity, "StaticMesh");

                            if (meshCol && staticMesh && staticMesh->MeshResource && staticMesh->MeshResource->cpuMesh)
                            {
                                meshCol->meshData = staticMesh->MeshResource->cpuMesh;
                                std::cout << "[Editor] Auto-linked new StaticMesh to MeshCollider.\n";
                            }
                        }
                    }
                }
            }, true);

    ForEachComponent("Remove component", Components,
        [this](Entity entity, const char* name) {
            Signature signature = engineAPI->GetEntitySignature(entity);
            if (engineAPI->HasComponent(entity, name))
            {
                engineAPI->RemoveComponent(entity, name);
            }
        }, false);

   
    
    Signature signature = engineAPI->GetEntitySignature(entity);

    for (auto componentInfo : Components)
    {

        if (signature.test(engineAPI->GetComponentType(componentInfo->name)))
        {
            if (ImGui::CollapsingHeader(componentInfo->name, ImGuiTreeNodeFlags_DefaultOpen))
            {
                void* data = engineAPI->GetComponentForWrite(entity, componentInfo->name);

                if (data)
                {
                    for (auto& variable : componentInfo->variables)
                    {
                        void* varPtr = (char*)data + variable.offset;

                        RenderVariable(variable.name, variable.type->name, varPtr, data, componentInfo->name);
                    }
                }
            }
        }
    }
    }

	ImGui::End();
}

