#pragma once

#include "imgui/imgui.h"
#include "imgui/imnodes.h"
#include <variant>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "json/json.hpp"

using json = nlohmann::json;

struct Pin
{
    int id;
    std::string label;
    ImNodesPinShape shape;
	bool isUniform = false;

    enum Type { Input, Output } type;
    enum DataType { Float, Vec2, Vec3, Color } data_type = Float;

    // Data storage — for now we’ll keep it simple
    std::variant<float, glm::vec2, glm::vec3, glm::vec4> value = 0.0f;
    template<typename T>
    T& Get() { return std::get<T>(value); }

    template<typename T>
    const T& Get() const { return std::get<T>(value); }

    template<typename T>
    void Set(const T& v)
    {
        value = v;

        if constexpr (std::is_same_v<T, float>)
            data_type = Float;
        else if constexpr (std::is_same_v<T, glm::vec2>)
            data_type = Vec2;
        else if constexpr (std::is_same_v<T, glm::vec3>)
            data_type = Vec3;
        else if constexpr (std::is_same_v<T, glm::vec4>)
            data_type = Color;
    }
};

struct Link
{
    int id;
    int start_pin_id;
    int end_pin_id;
};



struct BaseNode
{
    // Base properties...
    int id;
    std::string title;
    ImVec2 position;
    std::vector<Pin> Inputpins;
	std::vector<Pin> Outputpins;
    std::vector<Link> links;
	ImVec4 color = ImVec4(1.0f, 0.5f, 0.2f, 1.0f);
	ImVec4 titleColor = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);

    // Virtual function for the unique UI part
    virtual void DrawNodeContents() = 0;
    virtual json Serialize() const = 0;
    virtual void Deserialize(const json& data) = 0;

    virtual void Evaluate(std::vector<Link>&, std::vector<BaseNode*>&) = 0;

    virtual std::string GenerateShaderCode(const std::vector<Link>& links, const std::vector<BaseNode*>& nodes) = 0;

    // Final DrawNode wrapper
    void DrawNode()
    {
        ImNodes::PushColorStyle(ImNodesCol_TitleBar, ImGui::GetColorU32(titleColor));
        ImNodes::PushColorStyle(ImNodesCol_NodeBackground, ImGui::GetColorU32(color));
        ImNodes::BeginNode(id);

        ImNodes::BeginNodeTitleBar();
        ImGui::Text("%s", title.c_str());
        ImNodes::EndNodeTitleBar();


        for (const auto& pin : Inputpins)
        {
            ImNodes::BeginInputAttribute(pin.id, pin.shape);
            ImGui::Text("%s", pin.label.c_str());
            ImNodes::EndInputAttribute();
        }

        DrawNodeContents(); 

        for (const auto& pin : Outputpins)
        {
            ImNodes::BeginOutputAttribute(pin.id, pin.shape);
            ImGui::Text("%s", pin.label.c_str());
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();

		ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
    }

    // Virtual destructor for safety
    virtual ~BaseNode() = default;
};

inline Pin* FindLinkedPin(int pinId, const std::vector<Link>& links, std::vector<BaseNode*>& nodes)
{
    for (auto& link : links)
    {
        if (link.end_pin_id == pinId)
        {
            for (auto& node : nodes)
            {
                for (auto& outPin : node->Outputpins)
                {
                    if (outPin.id == link.start_pin_id)
                        return &outPin;
                }
            }
        }
    }
    return nullptr;
};

inline std::string GetConnectedVariableName(const Pin& inputPin,
    const std::vector<Link>& links,
    const std::vector<BaseNode*>& nodes)
{
    for (const auto& link : links)
    {
        if (link.end_pin_id == inputPin.id) // Input side
        {
            int outputPinId = link.start_pin_id;

            // Find the node that owns this pin
            for (auto* node : nodes)
            {
                for (auto& pin : node->Outputpins)
                {
                    if (pin.id == outputPinId)
                        return pin.label; // The GLSL variable name
                }
            }
        }
    }

    // No link ? use the fallback uniform label
    return inputPin.label;
}

struct AdderNode : public BaseNode
{
    AdderNode(int nodeId)
    {
        id = nodeId;
        title = "Add";
        color = ImVec4(0.4f, 0.8f, 0.4f, 1.0f);
        titleColor = ImVec4(0.1f, 0.5f, 0.1f, 1.0f);
        Inputpins.push_back({ id * 100 + 1, "A", ImNodesPinShape_Circle, Pin::Input });
        Inputpins.push_back({ id * 100 + 2, "B", ImNodesPinShape_Circle, Pin::Input });
        std::string outputLabel = "Add_R" + std::to_string(id);
        Outputpins.push_back({ id * 100 + 3, outputLabel, ImNodesPinShape_Circle, Pin::Output });
    }

    void Evaluate(std::vector<Link>& links, std::vector<BaseNode*>& nodes) override
    {
        Pin* aLink = FindLinkedPin(Inputpins[0].id, links, nodes);
        Pin* bLink = FindLinkedPin(Inputpins[1].id, links, nodes);

        float a = aLink ? aLink->Get<float>() : 0.0f;
        float b = bLink ? bLink->Get<float>() : 0.0f;

        Outputpins[0].Set<float>(a + b);
    }

    std::string GenerateShaderCode(const std::vector<Link>& links, const std::vector<BaseNode*>& nodes) override
    {
        std::string a = GetConnectedVariableName(Inputpins[0], links, nodes);
        std::string b = GetConnectedVariableName(Inputpins[1], links, nodes);
        // Use the unique label for the output variable
        return "    float " + Outputpins[0].label + " = " + a + " + " + b + ";\n";
    }

    void DrawNodeContents() override
    {
        ImGui::Text("Sum: %.2f", Outputpins[0].Get<float>());
    }

    json Serialize() const override
    {
        json j;
        j["type"] = "AddNode";
        j["id"] = id;
        j["position"] = { position.x, position.y };
        return j;
    }

    void Deserialize(const json& data) override
    {
        id = data["id"];
        position = ImVec2(data["position"][0], data["position"][1]);
    }
};

struct ConstantNode : public BaseNode
{
    ConstantNode(int nodeId)
    {
        id = nodeId;
        title = "Constant";
		color = ImVec4(0.4f, 0.8f, 0.4f, 1.0f);
		titleColor = ImVec4(0.1f, 0.5f, 0.1f, 1.0f);
        std::string outputLabel = "Const_V" + std::to_string(id);
        Outputpins.push_back({ id * 100 + 1, outputLabel, ImNodesPinShape_Circle, Pin::Output });
        Outputpins[0].Set<float>(1.0f);
    }

    void DrawNodeContents() override
    {
        float val = Outputpins[0].Get<float>();
        ImGui::SetNextItemWidth(120);
        if (ImGui::DragFloat("Value", &val, 0.01f))
            Outputpins[0].Set(val);
    }

    void Evaluate(std::vector<Link>&, std::vector<BaseNode*>&) override
    {
        // Nothing to compute, the value stays constant.
    }

    std::string GenerateShaderCode(const std::vector<Link>& links, const std::vector<BaseNode*>& nodes) override
    {
        float v = std::get<float>(Outputpins[0].value);
        auto s = std::to_string(v);
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') s.push_back('0');
        // Use the unique label for the output variable
        return "    float " + Outputpins[0].label + " = " + s + ";\n";
    }

    json Serialize() const override
    {
        json j;
        j["type"] = "ConstantNode";
        j["id"] = id;
        j["value"] = Outputpins[0].Get<float>();
        j["position"] = { position.x, position.y };
        return j;
    }

    void Deserialize(const json& data) override
    {
        id = data["id"];
        position = ImVec2(data["position"][0], data["position"][1]);
        Outputpins[0].Set<float>(data["value"]);
    }
};

struct TextureSampleNode : public BaseNode
{
    std::string texturePath;
    unsigned int textureId = 0;  // OpenGL texture handle (if loaded)
    bool textureLoaded = false;

    // Simulated outputs
    glm::vec4 colorValue = glm::vec4(1.0f);
    bool showChannels = true;

    TextureSampleNode(int nodeId)
    {
        id = nodeId;
        title = "Texture Sample";
        color = ImVec4(0.22f, 0.35f, 0.55f, 1.0f);
        titleColor = ImVec4(0.16f, 0.45f, 0.85f, 1.0f);

        // Input pin for UVs
        Inputpins.push_back({ id * 100 + 1, "UVs", ImNodesPinShape_Circle, Pin::Input});

        // --- CHANGE HERE: Unique label for the main vec4 output ---
        std::string outputLabel = "Tex_S" + std::to_string(id);
        Outputpins.push_back({ id * 100 + 2, outputLabel, ImNodesPinShape_Circle, Pin::Output }); // RGB (the vec4)

        // --- The channel pins use a standard name because they'll reference the main output ---
        // The GLSL generation will be smart enough to use the main unique var.
        Outputpins.push_back({ id * 100 + 3, "R",   ImNodesPinShape_Circle, Pin::Output });
        Outputpins.push_back({ id * 100 + 4, "G",   ImNodesPinShape_Circle, Pin::Output });
        Outputpins.push_back({ id * 100 + 5, "B",   ImNodesPinShape_Circle, Pin::Output });
        Outputpins.push_back({ id * 100 + 6, "A",   ImNodesPinShape_Circle, Pin::Output });

        // Set initial outputs
        Outputpins[0].Set<glm::vec4>(colorValue);
        Outputpins[1].Set<float>(colorValue.r);
        Outputpins[2].Set<float>(colorValue.g);
        Outputpins[3].Set<float>(colorValue.b);
        Outputpins[4].Set<float>(colorValue.a);
    }

    void DrawNodeContents() override
    {
        // Section: texture asset
        ImGui::Text("Texture:");
        ImGui::SameLine();

        char buf[256];
        strncpy_s(buf, texturePath.c_str(), sizeof(buf));
        ImGui::SetNextItemWidth(160);
        if (ImGui::InputText("##TexPath", buf, sizeof(buf)))
        {
            texturePath = buf;
            textureLoaded = false;
        }

        // Texture preview like Unreal
        ImVec2 previewSize(96, 96);
        if (textureLoaded && textureId != 0)
        {
            ImGui::Image((ImTextureID)(intptr_t)textureId, previewSize);
        }
        else
        {
            ImGui::Dummy(previewSize);
            ImGui::SameLine();
            ImGui::TextDisabled("[No Texture]");
        }

        // Show color output (simulated)
        glm::vec4 c = Outputpins[0].Get<glm::vec4>();
        ImGui::Text("Preview:");
        ImGui::SameLine();
        ImGui::ColorButton("ColorPreview", ImVec4(c.r, c.g, c.b, c.a),
            ImGuiColorEditFlags_NoTooltip, ImVec2(30, 30));
    }

    void Evaluate(std::vector<Link>& links, std::vector<BaseNode*>& nodes) override
    {
        // Unreal would sample based on UV input
        Pin* uvPin = FindLinkedPin(Inputpins[0].id, links, nodes);
        glm::vec3 uv = uvPin ? uvPin->Get<glm::vec3>() : glm::vec3(0.0f);

        // For now, we simulate a "texture sample" by hashing UV + path
        float seed = uv.x + uv.y + uv.z;
        float hash = 0.0f;
        for (char c : texturePath)
            hash += (float)c * 0.002f;
        glm::vec4 fakeColor = glm::vec4(
            fmod(hash + seed * 0.37f, 1.0f),
            fmod(hash + seed * 0.59f, 1.0f),
            fmod(hash + seed * 0.83f, 1.0f),
            1.0f
        );

        Outputpins[0].Set<glm::vec4>(fakeColor);
        Outputpins[1].Set<float>(fakeColor.r);
        Outputpins[2].Set<float>(fakeColor.g);
        Outputpins[3].Set<float>(fakeColor.b);
        Outputpins[4].Set<float>(fakeColor.a);
    }

    std::string GenerateShaderCode(const std::vector<Link>& links, const std::vector<BaseNode*>& nodes) override
    {
        glm::vec4 v = std::get<glm::vec4>(Outputpins[0].value);
        return "    vec4 " + Outputpins[0].label + " = vec4("
            + std::to_string(v.x) + ", "
            + std::to_string(v.y) + ", "
            + std::to_string(v.z) + ", "
            + std::to_string(v.w) + ");\n";
    }

    json Serialize() const override
    {
        json j;
        j["type"] = "TextureSampleNode";
        j["id"] = id;
        j["position"] = { position.x, position.y };
        j["texturePath"] = texturePath;
        return j;
    }

    void Deserialize(const json& data) override
    {
        id = data["id"];
        position = ImVec2(data["position"][0], data["position"][1]);
        texturePath = data.value("texturePath", "");
        textureLoaded = false;
    }
};

struct OutputNode : public BaseNode
{
    OutputNode(int nodeId)
    {
        id = nodeId;
        title = "Material Output";
        color = ImVec4(0.8f, 0.3f, 0.2f, 1.0f);
        titleColor = ImVec4(0.9f, 0.4f, 0.3f, 1.0f);

        // Default pins / PBR Material Model
        Inputpins.push_back({ id * 100 + 0, "WorldPosition", ImNodesPinShape_Circle, Pin::Input});
        Inputpins.push_back({ id * 100 + 1, "BaseColor", ImNodesPinShape_Circle, Pin::Input});
        Inputpins.push_back({ id * 100 + 2, "Emissive",  ImNodesPinShape_Circle, Pin::Input});
        Inputpins.push_back({ id * 100 + 3, "Opacity",   ImNodesPinShape_Circle, Pin::Input});
        Inputpins.push_back({ id * 100 + 4, "Metallic",  ImNodesPinShape_Circle, Pin::Input});
        Inputpins.push_back({ id * 100 + 5, "Roughness", ImNodesPinShape_Circle, Pin::Input});
        Inputpins.push_back({ id * 100 + 6, "Normal",    ImNodesPinShape_Circle, Pin::Input});

        // No outputs because this is THE end output node
    }

    void DrawNodeContents() override
    {
        ImGui::Text("Final Pixel Output");
    }

    void Evaluate(std::vector<Link>&, std::vector<BaseNode*>&) override
    {
        // Output node does no math — just forwards into shader
    }

    std::string GenerateShaderCode(const std::vector<Link>& links,
        const std::vector<BaseNode*>& nodes) override
    {
        // The input pin *labels* for the final output (e.g., BaseColor)
        // are used here to get the variable name that is connected to them.

        // Note: WorldPosition is unused in this simplified model, so we skip index 0.
        auto baseColorVar = GetConnectedVariableName(Inputpins[1], links, nodes);
        auto emissiveVar = GetConnectedVariableName(Inputpins[2], links, nodes);
        auto opacityVar = GetConnectedVariableName(Inputpins[3], links, nodes);

        std::string code = "";

        // Simplistic PBR-ish composition, assuming the inputs are floats or vec4s/vec3s as needed
        // This is where you need to be type-aware. Since we don't have a GLSL type system here, 
        // we make assumptions for this example:

        // BaseColor: Assume connected variable is a float/vec3/vec4. Use .rgb to be safe.
        code += "    vec3 base = " + baseColorVar + ".rgb;\n";

        // Emissive: Assume connected variable is a float/vec3/vec4. Use .rgb.
        code += "    vec3 emissive = " + emissiveVar + ".rgb;\n";

        // Opacity: Assume connected variable is a float.
        // If the variable is a vec4 (e.g. from TextureSample), GetConnectedVariableName will return
        // the variable name, and you need to append .a, .r, or .g etc. here.
        // For *this* example, let's assume `GetConnectedVariableName` returns a component
        // (float) if connected to a float-producing pin, and we manually clamp it:
        code += "    float opacity = clamp(" + opacityVar + ", 0.0, 1.0);\n";


        // Final composition
        code += "    vec3 finalColor = base + emissive;\n";
        code += "    finalColor = max(finalColor, 0.0);\n";
        code += "    FragColor = vec4(finalColor, opacity);\n";

        return code;
    }

    json Serialize() const override
    {
        json j;
        j["type"] = "OutputNode";
        j["id"] = id;
        j["position"] = { position.x, position.y };
        return j;
    }

    void Deserialize(const json& data) override
    {
        id = data["id"];
        position = ImVec2(data["position"][0], data["position"][1]);
    }
};
