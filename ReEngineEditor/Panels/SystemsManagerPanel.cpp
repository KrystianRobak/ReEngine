#include "SystemsManagerPanel.h"

#include "ReflectionEngine.h"

void SystemsManagerPanel::Render()
{
    ImGui::Begin("Systems Manager");

    for (const auto& system : Reflection::Registry::Instance().GetAllSystems())
    {
        if (ImGui::CollapsingHeader(system->fullName))
        {
            // 1. GET THE ACTUAL INSTANCE
            // We need the 'this' pointer to read variables. 
            // Assuming GetSystem returns void* or a pointer to the specific system.
            void* systemInstance = engineAPI->GetSystem(system->fullName);

            if (!systemInstance) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: System instance not found!");
                continue;
            }

            ImGui::TextDisabled("Variables");
            ImGui::Separator();

            // 2. ITERATE VARIABLES
            for (auto& val : system->variables)
            {
                // 3. CALCULATE VARIABLE MEMORY ADDRESS
                // We must cast to (uint8_t*) or (char*) before adding offset to do byte-wise arithmetic.
                void* varPtr = static_cast<uint8_t*>(systemInstance) + val.offset;

                // 4. DISPLAY BASED ON TYPE

                // Handle GLuint (Textures)
                if (std::strcmp(val.type->name, "unsigned int") == 0)
                {
                    GLuint texID = *static_cast<GLuint*>(varPtr);
                    ImGui::Text("%s (Texture):", val.name);
                    ImGui::Image((ImTextureID)(intptr_t)texID, ImVec2(100, 100));
                }
                // Handle Int
                else if (std::strcmp(val.type->name, "int") == 0 || std::strcmp(val.type->name, "uint32_t") == 0)
                {
                    // Direct pointer access allows ImGui to modify the value directly in memory!
                    int* valPtrInt = static_cast<int*>(varPtr);
                    ImGui::InputInt(val.name, valPtrInt);
                }
                // Handle Float
                else if (std::strcmp(val.type->name, "float") == 0)
                {
                    float* valPtrFloat = static_cast<float*>(varPtr);
                    ImGui::InputFloat(val.name, valPtrFloat);
                }
                // Handle Bool
                else if (std::strcmp(val.type->name, "bool") == 0)
                {
                    bool* valPtrBool = static_cast<bool*>(varPtr);
                    ImGui::Checkbox(val.name, valPtrBool);
                }
                // Handle String (Read-only for simplicity, editing std::string in ImGui requires resizing logic)
                else if (std::strcmp(val.type->name, "std::string") == 0)
                {
                    std::string* valPtrStr = static_cast<std::string*>(varPtr);
                    ImGui::Text("%s: %s", val.name, valPtrStr->c_str());
                }
                // Fallback
                else
                {
                    ImGui::Text("%s: <%s> (Unimplemented)", val.name, val.type->name);
                }
            }

            ImGui::Spacing();
            ImGui::TextDisabled("Functions");
            ImGui::Separator();
            // List all functions
            for (auto& func : system->functions)
            {
                ImGui::Text("Function: %s", func.name);

                static std::unordered_map<std::string, std::vector<std::string>> inputCache;

                // Ensure cache for this function
                auto& cache = inputCache[std::string(system->fullName) + "::" + func.name];
                if (cache.size() != func.paramTypes.size())
                    cache.resize(func.paramTypes.size());

                // Draw input fields for parameters
                for (size_t i = 0; i < func.paramTypes.size(); ++i)
                {
                    auto* type = func.paramTypes[i];
                    std::string label = std::string("Arg ") + std::to_string(i) + " (" + type->name + ")";

                    if (type->category == Reflection::TypeCategory::Unknown)
                    {
                        if (strcmp(type->name, "int") == 0 || strcmp(type->name, "uint32_t") == 0 || strcmp(type->name, "Entity") == 0)
                        {
                            int val = cache[i].empty() ? 0 : std::stoi(cache[i]);
                            if (ImGui::InputInt(label.c_str(), &val))
                                cache[i] = std::to_string(val);
                        }
                        else if (strcmp(type->name, "float") == 0)
                        {
                            float val = cache[i].empty() ? 0.0f : std::stof(cache[i]);
                            if (ImGui::InputFloat(label.c_str(), &val))
                                cache[i] = std::to_string(val);
                        }
                        else if (strcmp(type->name, "bool") == 0)
                        {
                            bool val = cache[i] == "1";
                            if (ImGui::Checkbox(label.c_str(), &val))
                                cache[i] = val ? "1" : "0";
                        }
                        else
                        {
                            char buf[128];
                            strncpy_s(buf, cache[i].c_str(), sizeof(buf));
                            buf[sizeof(buf) - 1] = 0;
                            if (ImGui::InputText(label.c_str(), buf, sizeof(buf)))
                                cache[i] = buf;
                        }
                    }
                    else
                    {
                        ImGui::Text("Unsupported type: %s", type->name);
                    }
                }

                // Call button
                if (ImGui::Button((std::string("Call ") + func.name).c_str()))
                {
                    std::vector<void*> args(func.paramTypes.size(), nullptr);
                    std::vector<int> intArgs;
                    std::vector<float> floatArgs;
                    std::vector<char> boolArgs;
                    std::vector<std::string> stringArgs;

                    // Convert strings to real values
                    for (size_t i = 0; i < func.paramTypes.size(); ++i)
                    {
                        auto* type = func.paramTypes[i];
                        if (strcmp(type->name, "int") == 0 || strcmp(type->name, "uint32_t") == 0 || strcmp(type->name, "Entity") == 0)
                        {
                            intArgs.push_back(std::stoi(cache[i]));
                            args[i] = &intArgs.back();
                        }
                        else if (strcmp(type->name, "float") == 0)
                        {
                            floatArgs.push_back(std::stof(cache[i]));
                            args[i] = &floatArgs.back();
                        }
                        else if (strcmp(type->name, "bool") == 0)
                        {
                            boolArgs.push_back(cache[i] == "1" ? 1 : 0);
                            args[i] = &boolArgs.back();
                        }
                        else
                        {
                            stringArgs.push_back(cache[i]);
                            args[i] = (void*)stringArgs.back().c_str();
                        }
                    }

                    const auto& SystemInstance = engineAPI->GetSystem(system->fullName);

                    void* ret = nullptr;
                    func.invoke(SystemInstance, args.data(), ret);
                }
            }
        }
    }

    ImGui::End();
}

