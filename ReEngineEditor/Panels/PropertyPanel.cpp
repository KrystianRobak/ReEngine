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

inline std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;

}

void RenderComponentsMenu(std::int32_t& entity, std::bitset<32>& signature) 
{

}

void PropertyPanel::OnInit()
{

}

void PropertyPanel::ForEachComponent(const char* header, std::vector<const Reflection::ClassInfo*> Components, std::function<void(Entity, const char*)> function)
{
    if (ImGui::CollapsingHeader(header))
    {
        for (auto component : Components)
        {
            Entity entity = engineAPI->GetSelectedEntity();

            if (ImGui::Button(component->name))
            {
                function(entity, component->name);
            }
            
        }
    }
}


void PropertyPanel::Render()
{
	ImGui::Begin("Properties");

    auto Components = Reflection::Registry::Instance().GetAllComponents();

    ForEachComponent("Add component", Components,
        [this](Entity entity, const char* name) {
            Signature signature = engineAPI->GetEntitySignature(entity);
            if (!signature.test(engineAPI->GetComponentType(name)))
            {
                engineAPI->AddComponent(entity, name);
            }
        });

    ForEachComponent("Remove component", Components,
        [this](Entity entity, const char* name) {
            Signature signature = engineAPI->GetEntitySignature(entity);
            if (signature.test(engineAPI->GetComponentType(name)))
            {
                engineAPI->RemoveComponent(entity, name);
            }
        });

    Entity entity = engineAPI->GetSelectedEntity();
    if (entity != 111)
    {
        Signature signature = engineAPI->GetEntitySignature(entity);

        for (auto componentInfo : Components) {
            // Check if the current entity has this component.
            if (signature.test(engineAPI->GetComponentType(componentInfo->name))) {
                // If it does, render a collapsible header for it.
                if (ImGui::CollapsingHeader(componentInfo->name)) {
                    // Get the actual component data pointer.
                    // 1. Get BOTH pointers
                    void* readData = engineAPI->GetComponent(entity, componentInfo->name);
                    void* writeData = engineAPI->GetComponentForWrite(entity, componentInfo->name);

                    // Check if we actually have two different buffers (Double Buffering Active)
                    bool isDoubleBuffered = (readData != writeData && writeData != nullptr);

                    for (auto& variable : componentInfo->variables) {
                        char* writeVarPtr = (char*)writeData + variable.offset;

                        const char* typeName = variable.type->name;
                        const char* varName = variable.name;

                        bool valueChanged = false;

                        // --- Render UI based on READ pointer ---
                        // We use the Read pointer for the UI so the user sees the current frame's state
                        if (strcmp(typeName, "float") == 0) {
                            if (ImGui::DragFloat(varName, (float*)writeVarPtr, 0.1f)) valueChanged = true;
                        }
                        else if (strcmp(typeName, "int") == 0) {
                            if (ImGui::InputInt(varName, (int*)writeVarPtr)) valueChanged = true;
                        }
                        else if (strcmp(typeName, "bool") == 0) {
                            if (ImGui::Checkbox(varName, (bool*)writeVarPtr)) valueChanged = true;
                        }
                        else if (strcmp(typeName, "glm::vec<3, float>") == 0 || strcmp(typeName, "glm::vec3") == 0) {
                            if (ImGui::DragFloat3(varName, (float*)writeVarPtr, 0.1f)) valueChanged = true;
                        }
                        else if (strcmp(typeName, "glm::vec<4, float>") == 0 || strcmp(typeName, "glm::vec4") == 0) {
                            if (ImGui::DragFloat4(varName, (float*)writeVarPtr, 0.1f)) valueChanged = true;
                        }
                        else if (strcmp(typeName, "glm::qua<float>") == 0) {
                            if (ImGui::DragFloat4(varName, (float*)writeVarPtr, 0.1f)) valueChanged = true;
                        }
                        else if (strcmp(typeName, "std::string") == 0 || strcmp(typeName, "std::basic_string<char>") == 0) {
                            std::string* strPtr = (std::string*)writeVarPtr;

                            // 1. Static Mesh Asset
                            if (strcmp(componentInfo->name, "StaticMesh") == 0 && strcmp(varName, "AssetPath") == 0) {
                                if (DrawAssetSlot(varName, *strPtr, "ASSET_STATIC_MESH", FileType::StaticMesh)) {
                                    // FORCE UPDATE: Clear the resource handle so the System re-fetches it next frame
                                    ((StaticMesh*)writeData)->MeshResource = engineAPI->GetAssetManager()->GetMesh(*strPtr);
                                    //engineAPI->MarkEntityDirty(entity, componentInfo->name);
                                }
                            }
                            // 2. Skeletal Mesh Asset
                            else if (strcmp(componentInfo->name, "SkeletalMeshComponent") == 0 && strcmp(varName, "AssetPath") == 0) {
                                if (DrawAssetSlot(varName, *strPtr, "ASSET_SKELETAL_MESH", FileType::SkeletalMesh)) {
                                    // FORCE UPDATE
                                    ((SkeletalMeshComponent*)writeData)->MeshResource = engineAPI->GetAssetManager()->GetMesh(*strPtr);
                                    //engineAPI->MarkEntityDirty(entity, componentInfo->name);
                                }
                            }
                            // 3. Animation Graph (State Machine)
                            else if (strcmp(componentInfo->name, "StateMachine") == 0 && strcmp(varName, "GraphAssetPath") == 0) {
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
                    }

                }
            }
        }
    }

	ImGui::End();
}

