#pragma once

#include "imgui/imgui.h"
#include "imgui/imnodes.h"
#include <variant>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "json/json.hpp"
#include "Api/AssetManagerApi.h"
#include "TextureData.h"

using json = nlohmann::json;



struct Pin
{
    int id;
    std::string label;
    ImNodesPinShape shape;
    bool isUniform = false;

    enum Type { Input, Output } type;
    enum DataType { Float, Vec2, Vec3, Color } data_type = Float;

    // Data storage � for now we�ll keep it simple
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

    Pin(int _id, const std::string& _label, ImNodesPinShape _shape, Type _type,
        DataType _dtype = Float, bool _isUniform = false)
        : id(_id), label(_label), shape(_shape), isUniform(_isUniform),
        type(_type), data_type(_dtype) {
    }
};

struct Link
{
    int id;
    int start_pin_id;
    int end_pin_id;
};

inline const char* GLSLType(Pin::DataType type) {
    switch (type) {
    case Pin::Float: return "float";
    case Pin::Vec2:  return "vec2";
    case Pin::Vec3:  return "vec3";
    case Pin::Color: return "vec4";
    }
    return "float";
}

inline std::string GetDefaultValueForInput(const Pin& pin)
{
    if (pin.label == "BaseColor")  return "vec3(1.0)";
    if (pin.label == "Emissive")   return "vec3(0.0)";
    if (pin.label == "Opacity")    return "1.0";
    if (pin.label == "Metallic")   return "0.0";
    if (pin.label == "Roughness")  return "1.0";
    if (pin.label == "Normal")     return "vec3(0.5, 0.5, 1.0)";
    if (pin.label == "WorldPosition") return "FragPos";
    if (pin.label == "UVs")        return "TexCoords";
    return "0.0";
}

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

    void UpdatePinIds()
    {
        for (auto& pin : Inputpins) {
            int localIndex = pin.id % 100; // Extract local offset (e.g. 1)
            pin.id = this->id * 100 + localIndex; // Rebuild ID: 501
        }
        for (auto& pin : Outputpins) {
            int localIndex = pin.id % 100; // Extract local offset (e.g. 2)
            pin.id = this->id * 100 + localIndex; // Rebuild ID: 502
        }
    }

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
    for (auto& l : links)
    {
        if (l.end_pin_id == inputPin.id)
        {
            for (auto* n : nodes)
            {
                for (auto& op : n->Outputpins)
                {
                    if (op.id == l.start_pin_id)
                        return op.label;
                }
            }
        }
    }

    // No link ? return default value for this pin
    return GetDefaultValueForInput(inputPin);
}

inline Pin::DataType PromoteType(Pin::DataType a, Pin::DataType b)
{
    return (a > b) ? a : b;
}

template<typename T>
T ZeroValue();

template<> inline float ZeroValue<float>() { return 0.0f; }
template<> inline glm::vec2 ZeroValue<glm::vec2>() { return glm::vec2(0.0f); }
template<> inline glm::vec3 ZeroValue<glm::vec3>() { return glm::vec3(0.0f); }
template<> inline glm::vec4 ZeroValue<glm::vec4>() { return glm::vec4(0.0f); }

template<typename T>
T GetConnectedValue(const Pin& pin,
    const std::vector<Link>& links,
    std::vector<BaseNode*>& nodes)
{
    if (Pin* p = FindLinkedPin(pin.id, links, nodes))
        return p->Get<T>();
    return ZeroValue<T>();
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

    Pin::DataType ResolveType(const std::vector<Link>& links, std::vector<BaseNode*>& nodes)
    {
        Pin* a = FindLinkedPin(Inputpins[0].id, links, nodes);
        Pin* b = FindLinkedPin(Inputpins[1].id, links, nodes);
        return PromoteType(a ? a->data_type : Pin::Float,
            b ? b->data_type : Pin::Float);
    }

    std::string GenerateShaderCode(const std::vector<Link>& links, const std::vector<BaseNode*>& nodes) override
    {
        // BUG FIX #2: The original code unconditionally emitted "float X = a + b;"
        // regardless of the actual input types. If two vec3 colors were wired in, the
        // output was "float X = vec3(...) + vec3(...);" — a GLSL type error that
        // silently broke shader compilation the same way as Bug #1.
        // Fix: resolve the promoted type from connected pins, exactly like AddNode does.
        Pin::DataType t = ResolveType(links, (std::vector<BaseNode*>&)nodes);
        std::string typeStr = GLSLType(t);
        std::string a = GetConnectedVariableName(Inputpins[0], links, nodes);
        std::string b = GetConnectedVariableName(Inputpins[1], links, nodes);
        return "    " + typeStr + " " + Outputpins[0].label + " = " + a + " + " + b + ";\n";
    }

    void DrawNodeContents() override
    {
        ImGui::Text("Add (%s)", GLSLType(Outputpins[0].data_type));
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
        UpdatePinIds();
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
        UpdatePinIds();
    }
};

struct TextureSampleNode : public BaseNode
{
    std::string texturePath;
    unsigned int textureId = 0;  // OpenGL texture handle (if loaded)
    bool textureLoaded = false;
    AssetManagerApi* assetManager = nullptr; // BUG FIX #4: was uninitialized — null-pointer crash in DrawNodeContents if Init() never called
    // Simulated outputs
    glm::vec4 colorValue = glm::vec4(1.0f);
    bool showChannels = true;

    void Init(AssetManagerApi* am)
    {
        assetManager = am;
    }

    TextureSampleNode(int nodeId)
    {
        id = nodeId;
        title = "Texture Sample";
        color = ImVec4(0.22f, 0.35f, 0.55f, 1.0f);
        titleColor = ImVec4(0.16f, 0.45f, 0.85f, 1.0f);

        // Input pin for UVs
        Inputpins.push_back({ id * 100 + 1, "UVs", ImNodesPinShape_Circle, Pin::Input });

        // Main Output
        std::string outputLabel = "Tex_S" + std::to_string(id);
        // BUG FIX #1: data_type was omitted, defaulting to Pin::Float.
        // GenerateShaderCode emits "vec4 Tex_S<id> = texture(...)" — the GLSL type IS
        // vec4. OutputNode::ResolveInput uses data_type to decide whether to apply
        // ".rgb"/".r" swizzles. With Float, no swizzle was applied, so scalar inputs
        // (Metallic, Roughness) got code like "gPosition.a = Tex_S5;" — assigning
        // vec4 to float — a GLSL compile error. The program compiled to ID 0,
        // glUseProgram(0) ran during the GBuffer pass, the attachments were never
        // written, and the lighting pass read WorldPos=(0,0,0) for those entities,
        // producing incorrect shadow lookups on all material-shaded geometry.
        Outputpins.push_back({ id * 100 + 2, outputLabel, ImNodesPinShape_Circle, Pin::Output, Pin::Color }); // vec4

        // Channel outputs: correctly typed as Float (each is a scalar component)
        Outputpins.push_back({ id * 100 + 3, outputLabel + "_R", ImNodesPinShape_Circle, Pin::Output, Pin::Float });
        Outputpins.push_back({ id * 100 + 4, outputLabel + "_G", ImNodesPinShape_Circle, Pin::Output, Pin::Float });
        Outputpins.push_back({ id * 100 + 5, outputLabel + "_B", ImNodesPinShape_Circle, Pin::Output, Pin::Float });
        Outputpins.push_back({ id * 100 + 6, outputLabel + "_A", ImNodesPinShape_Circle, Pin::Output, Pin::Float });

        // Set initial outputs
        Outputpins[0].Set<glm::vec4>(colorValue);
        Outputpins[1].Set<float>(colorValue.r);
        Outputpins[2].Set<float>(colorValue.g);
        Outputpins[3].Set<float>(colorValue.b);
        Outputpins[4].Set<float>(colorValue.a);
    }

    void DrawNodeContents() override
    {
        // --------------------------
        // Texture Picker (with Thumbnail)
        // --------------------------
        // BUG FIX #4 (continued): Guard against null assetManager.
        // TextureSampleNode::assetManager was uninitialized (no default value) and
        // Init() is only called on editor-constructed nodes, not on deserialized ones.
        // Calling GetCachedTexturesPaths() through a garbage pointer is a crash.
        if (!assetManager) {
            ImGui::TextDisabled("[Asset manager not initialized]");
            return;
        }

        ImGui::Text("Texture:");
        ImGui::SameLine();

        std::vector<std::string> cached = assetManager->GetCachedTexturesPaths();
        const char* previewName = texturePath.empty() ? "<None>" : texturePath.c_str();

        ImGui::SetNextItemWidth(160);
        if (ImGui::BeginCombo("##TexturePicker", previewName))
        {
            for (const std::string& path : cached)
            {
                bool selected = (texturePath == path);
                ImGui::PushID(path.c_str());

                ImGui::BeginGroup();

                // Get texture resource for thumbnail
                auto res = assetManager->GetTexture(path);

                // Clickable whole row
                ImGui::Selectable("##sel", selected, 0, ImVec2(0, 48));
                ImGui::SameLine();

                if (res && res->uploaded && res->id != 0)
                {
                    ImGui::Image((ImTextureID)(intptr_t)res->id,
                        ImVec2(48, 48), ImVec2(0, 1), ImVec2(1, 0));
                }
                else
                {
                    ImGui::Dummy(ImVec2(48, 48));
                }

                ImGui::SameLine();
                std::string displayName = path;

                // 1. Find the last path separator (supports both / and \)
                size_t lastSlash = path.find_last_of("/\\");
                if (lastSlash != std::string::npos) {
                    // Extract the filename (everything after the last slash)
                    displayName = path.substr(lastSlash + 1);
                }

                // 2. Find the last dot in the extracted filename
                size_t lastDot = displayName.find_last_of('.');
                if (lastDot != std::string::npos) {
                    // Remove the extension
                    displayName = displayName.substr(0, lastDot);
                }

                ImGui::Text("%s", displayName.c_str());

                ImGui::EndGroup();

                if (ImGui::IsItemClicked())
                {
                    texturePath = path;
                    textureLoaded = false;
                }

                if (selected)
                    ImGui::SetItemDefaultFocus();

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        // --------------------------
        // Load + Retrieve GPU texture
        // --------------------------
        if (!textureLoaded && !texturePath.empty())
        {
            auto res = assetManager->GetTexture(texturePath);
            if (res && res->uploaded)
            {
                textureId = res->id;
                textureLoaded = true;
            }
            else
            {
                // Request async load
                assetManager->GetTexture(texturePath);
            }
        }

        // --------------------------
        // Preview
        // --------------------------
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

        // --------------------------
        // Color Preview (fake for now)
        // --------------------------
        glm::vec4 c = Outputpins[0].Get<glm::vec4>();
        ImGui::Text("Preview:");
        ImGui::SameLine();
        ImGui::ColorButton("ColorPreview",
            ImVec4(c.r, c.g, c.b, c.a),
            ImGuiColorEditFlags_NoTooltip,
            ImVec2(30, 30));
    }

    void Evaluate(std::vector<Link>& links, std::vector<BaseNode*>& nodes) override
    {
        // Unreal would sample based on UV input
        Pin* uvPin = FindLinkedPin(Inputpins[0].id, links, nodes);
        glm::vec2 uv = uvPin ? uvPin->Get<glm::vec2>() : glm::vec2(0.0f);

        // For now, we simulate a "texture sample" by hashing UV + path
        float seed = uv.x + uv.y;
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

    std::string GenerateShaderCode(const std::vector<Link>& links,
        const std::vector<BaseNode*>& nodes) override
    {
        std::string uv = GetConnectedVariableName(Inputpins[0], links, nodes);

        std::string samplerName = Outputpins[0].label + "_Tex";

        std::string code;
        code += "    vec4 " + Outputpins[0].label +
            " = texture(" + samplerName + ", " + uv + " );\n";

        // scalar channels
        code += "    float " + Outputpins[1].label + " = " + Outputpins[0].label + ".r;\n";
        code += "    float " + Outputpins[2].label + " = " + Outputpins[0].label + ".g;\n";
        code += "    float " + Outputpins[3].label + " = " + Outputpins[0].label + ".b;\n";
        code += "    float " + Outputpins[4].label + " = " + Outputpins[0].label + ".a;\n";

        return code;
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
        UpdatePinIds();
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

        // BUG FIX #5: WorldPosition pin used offset 0 (id * 100 + 0 = id * 100).
        // While that doesn't cause a numerical collision, it is unsafe in UpdatePinIds
        // (localIndex = pin.id % 100 = 0 would alias with any other "base" id).
        // More importantly, GenerateShaderCode completely ignores this pin and always
        // hardcodes "gPosition.rgb = FragPos;" — so any connection was silently dropped.
        // Changed to offset 7 (one past Normal) to make the ID scheme consistent, and
        // the pin is now visually marked "(not yet implemented)" so users know it's a
        // placeholder for future world-position offset support.
        // Inputs: 1:Color, 2:Emissive, 3:Opacity, 4:Metallic, 5:Roughness, 6:Normal, 7:WorldPosition(NYI)
        Inputpins.push_back({ id * 100 + 1, "BaseColor", ImNodesPinShape_Circle, Pin::Input });
        Inputpins.push_back({ id * 100 + 2, "Emissive",  ImNodesPinShape_Circle, Pin::Input });
        Inputpins.push_back({ id * 100 + 3, "Opacity",   ImNodesPinShape_Circle, Pin::Input });
        Inputpins.push_back({ id * 100 + 4, "Metallic",  ImNodesPinShape_Circle, Pin::Input });
        Inputpins.push_back({ id * 100 + 5, "Roughness", ImNodesPinShape_Circle, Pin::Input });
        Inputpins.push_back({ id * 100 + 6, "Normal",    ImNodesPinShape_Circle, Pin::Input });
        Inputpins.push_back({ id * 100 + 7, "WorldPos (NYI)", ImNodesPinShape_CircleFilled, Pin::Input });
    }

    void DrawNodeContents() override
    {
        ImGui::Text("Final Pixel Output");
    }

    void Evaluate(std::vector<Link>&, std::vector<BaseNode*>&) override
    {
        // Output node does no math � just forwards into shader
    }

    // Helper to safely cast variables based on connection types
    std::string ResolveInput(int pinIndex, const std::vector<Link>& links, const std::vector<BaseNode*>& nodes, const std::string& targetType)
    {
        std::string varName = GetConnectedVariableName(Inputpins[pinIndex], links, nodes);
        Pin* connectedPin = FindLinkedPin(Inputpins[pinIndex].id, links, (std::vector<BaseNode*>&)nodes);

        if (connectedPin)
        {
            // Case 1: Target is FLOAT, but Input is COLOR (Vec4) -> Use Red Channel
            if (targetType == "float" && connectedPin->data_type == Pin::Color) {
                return varName + ".r";
            }
            // Case 2: Target is VEC3, but Input is COLOR (Vec4) -> Use RGB
            if (targetType == "vec3" && connectedPin->data_type == Pin::Color) {
                return varName + ".rgb";
            }
        }
        return varName;
    }

    std::string GenerateShaderCode(const std::vector<Link>& links,
        const std::vector<BaseNode*>& nodes) override
    {
        // 1. Fetch variable names with Auto-Swizzling
        // Indices match Inputpins order after BUG FIX #5 (WorldPosition moved to last, NYI):
        // 0=BaseColor, 1=Emissive, 2=Opacity, 3=Metallic, 4=Roughness, 5=Normal, 6=WorldPos(NYI)
        std::string baseColor = ResolveInput(0, links, nodes, "vec3");
        std::string opacity = ResolveInput(2, links, nodes, "float");
        std::string metallic = ResolveInput(3, links, nodes, "float");
        std::string roughness = ResolveInput(4, links, nodes, "float");
        std::string normalIn = ResolveInput(5, links, nodes, "vec3");

        std::string code;

        // --- ATTACHMENT 0: Position + Metallic ---
        code += "    gPosition.rgb = FragPos;\n";
        code += "    gPosition.a = " + metallic + ";\n";

        // --- ATTACHMENT 1: Normal ---
        // Wrap in vec3() constructor to be safe against float inputs
        code += "    vec3 mapNormal = vec3(" + normalIn + ");\n";

        // Safety: If mapNormal is essentially zero/default, fall back to geometry normal
        code += "    if (length(mapNormal) < 0.1) mapNormal = vec3(0.0, 0.0, 1.0);\n";

        // Convert from [0,1] texture range to [-1,1] vector range
        code += "    mapNormal = normalize(mapNormal * 2.0 - 1.0);\n";

        // Apply TBN matrix to transform Tangent Space -> World Space
        code += "    gNormal.rgb = normalize(TBN * mapNormal);\n";
        code += "    gNormal.a = 1.0;\n";

        // --- ATTACHMENT 2: Albedo + Roughness ---
        code += "    gAlbedoSpec.rgb = vec3(" + baseColor + ");\n";
        code += "    gAlbedoSpec.a = " + roughness + ";\n";

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
        UpdatePinIds();
    }
};


// BUG FIX #3: AddNode was a duplicate struct that also serialized as "type":"AddNode".
// Material::Deserialize always created AdderNode for that key — this struct was
// unreachable from disk, making it dead code. Having two structs with the same
// serialization string is a silent correctness trap; removed. AdderNode (now
// corrected with proper type-promotion by Bug Fix #2) is the single canonical Add node.
// Any editor code that called AddNode<AddNode>() should call AddNode<AdderNode>() instead.


struct ConstantVec2Node : public BaseNode
{
    ConstantVec2Node(int nodeId)
    {
        id = nodeId;
        title = "Vec2 Constant";
        color = ImVec4(0.4f, 0.7f, 0.9f, 1.0f);
        titleColor = ImVec4(0.1f, 0.4f, 0.7f, 1.0f);

        std::string oName = "ConstV2_" + std::to_string(id);
        Outputpins.push_back({ id * 100 + 1, oName, ImNodesPinShape_Circle, Pin::Output, Pin::Vec2 });
        Outputpins[0].Set<glm::vec2>(glm::vec2(0.0f));
    }

    void DrawNodeContents() override
    {
        glm::vec2 v = Outputpins[0].Get<glm::vec2>();
        if (ImGui::DragFloat2("Value", (float*)&v, 0.01f))
            Outputpins[0].Set<glm::vec2>(v);
    }

    void Evaluate(std::vector<Link>&, std::vector<BaseNode*>&) override {}

    std::string GenerateShaderCode(const std::vector<Link>&, const std::vector<BaseNode*>&) override
    {
        glm::vec2 v = Outputpins[0].Get<glm::vec2>();
        return "    vec2 " + Outputpins[0].label +
            " = vec2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ");\n";
    }

    json Serialize() const override
    {
        glm::vec2 v = Outputpins[0].Get<glm::vec2>();
        return {
            {"type","ConstantVec2Node"},
            {"id",id},
            {"value",{v.x, v.y}},
            {"position",{position.x,position.y}}
        };
    }

    void Deserialize(const json& j) override
    {
        id = j["id"];
        position = ImVec2(j["position"][0], j["position"][1]);
        Outputpins[0].Set<glm::vec2>({ j["value"][0], j["value"][1] });
        UpdatePinIds();
    }
};

struct ConstantVec3Node : public BaseNode
{
    ConstantVec3Node(int nodeId)
    {
        id = nodeId;
        title = "Vec3 Constant";
        color = ImVec4(0.4f, 0.7f, 0.9f, 1.0f);
        titleColor = ImVec4(0.1f, 0.4f, 0.7f, 1.0f);

        std::string oName = "ConstV3_" + std::to_string(id);
        Outputpins.push_back({ id * 100 + 1, oName, ImNodesPinShape_Circle, Pin::Output, Pin::Vec3 });
        Outputpins[0].Set<glm::vec3>(glm::vec3(0.0f));
    }

    void DrawNodeContents() override
    {
        glm::vec3 v = Outputpins[0].Get<glm::vec3>();
        if (ImGui::ColorEdit3("Value", (float*)&v))
            Outputpins[0].Set<glm::vec3>(v);
    }

    void Evaluate(std::vector<Link>&, std::vector<BaseNode*>&) override {}

    std::string GenerateShaderCode(const std::vector<Link>&, const std::vector<BaseNode*>&) override
    {
        glm::vec3 v = Outputpins[0].Get<glm::vec3>();
        return "    vec3 " + Outputpins[0].label +
            " = vec3(" + std::to_string(v.x) + ", " +
            std::to_string(v.y) + ", " +
            std::to_string(v.z) + ");\n";
    }

    json Serialize() const override
    {
        glm::vec3 v = Outputpins[0].Get<glm::vec3>();
        return {
            {"type","ConstantVec3Node"},
            {"id",id},
            {"value",{v.x, v.y, v.z}},
            {"position",{position.x,position.y}}
        };
    }

    void Deserialize(const json& j) override
    {
        id = j["id"];
        position = ImVec2(j["position"][0], j["position"][1]);
        Outputpins[0].Set<glm::vec3>({ j["value"][0], j["value"][1], j["value"][2] });
        UpdatePinIds();
    }
};

struct MultiplyNode : public BaseNode
{
    MultiplyNode(int nodeId)
    {
        id = nodeId;
        title = "Multiply";
        color = ImVec4(0.4f, 0.7f, 0.4f, 1.0f);
        titleColor = ImVec4(0.15f, 0.45f, 0.15f, 1.0f);

        Inputpins.push_back({ id * 100 + 1, "A", ImNodesPinShape_Circle, Pin::Input });
        Inputpins.push_back({ id * 100 + 2, "B", ImNodesPinShape_Circle, Pin::Input });

        std::string oName = "Mul_" + std::to_string(id);
        Outputpins.push_back({ id * 100 + 3, oName, ImNodesPinShape_Circle, Pin::Output });
    }

    Pin::DataType ResolveType(const std::vector<Link>& links, std::vector<BaseNode*>& nodes)
    {
        Pin* a = FindLinkedPin(Inputpins[0].id, links, nodes);
        Pin* b = FindLinkedPin(Inputpins[1].id, links, nodes);
        return PromoteType(a ? a->data_type : Pin::Float,
            b ? b->data_type : Pin::Float);
    }

    void Evaluate(std::vector<Link>& links, std::vector<BaseNode*>& nodes) override
    {
        Pin::DataType t = ResolveType(links, nodes);
        if (t == Pin::Float)
            Outputpins[0].Set<float>(GetConnectedValue<float>(Inputpins[0], links, nodes) *
                GetConnectedValue<float>(Inputpins[1], links, nodes));
        else if (t == Pin::Vec2)
            Outputpins[0].Set<glm::vec2>(GetConnectedValue<glm::vec2>(Inputpins[0], links, nodes) *
                GetConnectedValue<glm::vec2>(Inputpins[1], links, nodes));
        else if (t == Pin::Vec3)
            Outputpins[0].Set<glm::vec3>(GetConnectedValue<glm::vec3>(Inputpins[0], links, nodes) *
                GetConnectedValue<glm::vec3>(Inputpins[1], links, nodes));
        else
            Outputpins[0].Set<glm::vec4>(GetConnectedValue<glm::vec4>(Inputpins[0], links, nodes) *
                GetConnectedValue<glm::vec4>(Inputpins[1], links, nodes));

        Outputpins[0].data_type = t;
    }

    std::string GenerateShaderCode(const std::vector<Link>& links,
        const std::vector<BaseNode*>& nodes) override
    {
        Pin::DataType t = ResolveType(links, (std::vector<BaseNode*>&)nodes);
        std::string typeStr = GLSLType(t);

        std::string a = GetConnectedVariableName(Inputpins[0], links, nodes);
        std::string b = GetConnectedVariableName(Inputpins[1], links, nodes);

        return "    " + typeStr + " " + Outputpins[0].label + " = " + a + " * " + b + ";\n";
    }

    void DrawNodeContents() override
    {
        ImGui::Text("Multiply (%s)", GLSLType(Outputpins[0].data_type));
    }

    json Serialize() const override
    {
        return {
            {"type","MultiplyNode"},
            {"id",id},
            {"position",{position.x,position.y}}
        };
    }

    void Deserialize(const json& j) override
    {
        id = j["id"];
        position = ImVec2(j["position"][0], j["position"][1]);
        UpdatePinIds();
    }
};

struct DotNode : public BaseNode
{
    DotNode(int nodeId)
    {
        id = nodeId;
        title = "Dot";
        color = ImVec4(0.5f, 0.5f, 0.8f, 1.0f);

        Inputpins.push_back({ id * 100 + 1, "A", ImNodesPinShape_Circle, Pin::Input, Pin::Vec3 });
        Inputpins.push_back({ id * 100 + 2, "B", ImNodesPinShape_Circle, Pin::Input, Pin::Vec3 });

        std::string oName = "Dot_" + std::to_string(id);
        Outputpins.push_back({ id * 100 + 3, oName, ImNodesPinShape_Circle, Pin::Output, Pin::Float });
    }

    void Evaluate(std::vector<Link>& links, std::vector<BaseNode*>& nodes) override
    {
        glm::vec3 a = GetConnectedValue<glm::vec3>(Inputpins[0], links, nodes);
        glm::vec3 b = GetConnectedValue<glm::vec3>(Inputpins[1], links, nodes);
        Outputpins[0].Set<float>(glm::dot(a, b));
    }

    std::string GenerateShaderCode(const std::vector<Link>& links,
        const std::vector<BaseNode*>& nodes) override
    {
        std::string a = GetConnectedVariableName(Inputpins[0], links, nodes);
        std::string b = GetConnectedVariableName(Inputpins[1], links, nodes);
        return "    float " + Outputpins[0].label + " = dot(" + a + ", " + b + ");\n";
    }

    void DrawNodeContents() override
    {
    }

    json Serialize() const override
    {
        return {
            {"type","DotNode"},
            {"id",id},
            {"position",{position.x,position.y}}
        };
    }

    void Deserialize(const json& j) override
    {
        id = j["id"];
        position = ImVec2(j["position"][0], j["position"][1]);
        UpdatePinIds();
    }
};

struct LerpNode : public BaseNode
{
    LerpNode(int nodeId)
    {
        id = nodeId;
        title = "Lerp";
        color = ImVec4(0.6f, 0.45f, 0.35f, 1.0f);

        Inputpins.push_back({ id * 100 + 1, "A", ImNodesPinShape_Circle, Pin::Input });
        Inputpins.push_back({ id * 100 + 2, "B", ImNodesPinShape_Circle, Pin::Input });
        Inputpins.push_back({ id * 100 + 3, "T", ImNodesPinShape_Circle, Pin::Input, Pin::Float });

        std::string oName = "Lerp_" + std::to_string(id);
        Outputpins.push_back({ id * 100 + 4, oName, ImNodesPinShape_Circle, Pin::Output });
    }
    void DrawNodeContents() override
    {
    }
    Pin::DataType ResolveType(const std::vector<Link>& links, std::vector<BaseNode*>& nodes)
    {
        Pin* a = FindLinkedPin(Inputpins[0].id, links, nodes);
        Pin* b = FindLinkedPin(Inputpins[1].id, links, nodes);
        return PromoteType(a ? a->data_type : Pin::Float,
            b ? b->data_type : Pin::Float);
    }

    void Evaluate(std::vector<Link>& links, std::vector<BaseNode*>& nodes) override
    {
        float t = GetConnectedValue<float>(Inputpins[2], links, nodes);

        Pin::DataType type = ResolveType(links, nodes);

        if (type == Pin::Float)
        {
            float a = GetConnectedValue<float>(Inputpins[0], links, nodes);
            float b = GetConnectedValue<float>(Inputpins[1], links, nodes);
            Outputpins[0].Set<float>(a * (1.0f - t) + b * t);
        }
        else if (type == Pin::Vec2)
        {
            glm::vec2 a = GetConnectedValue<glm::vec2>(Inputpins[0], links, nodes);
            glm::vec2 b = GetConnectedValue<glm::vec2>(Inputpins[1], links, nodes);
            Outputpins[0].Set<glm::vec2>(a * (1.0f - t) + b * t);
        }
        else if (type == Pin::Vec3)
        {
            glm::vec3 a = GetConnectedValue<glm::vec3>(Inputpins[0], links, nodes);
            glm::vec3 b = GetConnectedValue<glm::vec3>(Inputpins[1], links, nodes);
            Outputpins[0].Set<glm::vec3>(a * (1.0f - t) + b * t);
        }
        else
        {
            glm::vec4 a = GetConnectedValue<glm::vec4>(Inputpins[0], links, nodes);
            glm::vec4 b = GetConnectedValue<glm::vec4>(Inputpins[1], links, nodes);
            Outputpins[0].Set<glm::vec4>(a * (1.0f - t) + b * t);
        }

        Outputpins[0].data_type = type;
    }

    std::string GenerateShaderCode(const std::vector<Link>& links,
        const std::vector<BaseNode*>& nodes) override
    {
        Pin::DataType t = ResolveType(links, (std::vector<BaseNode*>&)nodes);

        std::string typeStr = GLSLType(t);
        std::string a = GetConnectedVariableName(Inputpins[0], links, nodes);
        std::string b = GetConnectedVariableName(Inputpins[1], links, nodes);
        std::string factor = GetConnectedVariableName(Inputpins[2], links, nodes);

        return "    " + typeStr + " " + Outputpins[0].label +
            " = mix(" + a + ", " + b + ", " + factor + ");\n";
    }

    json Serialize() const override
    {
        return { {"type","LerpNode"},{"id",id},{"position",{position.x,position.y}} };
    }

    void Deserialize(const json& j) override
    {
        id = j["id"];
        position = ImVec2(j["position"][0], j["position"][1]);
        UpdatePinIds();
    }
};


struct CrossNode : public BaseNode
{
    CrossNode(int nodeId)
    {
        id = nodeId;
        title = "Cross";
        color = ImVec4(0.7f, 0.4f, 0.4f, 1.0f);

        Inputpins.push_back({ id * 100 + 1, "A", ImNodesPinShape_Circle, Pin::Input, Pin::Vec3 });
        Inputpins.push_back({ id * 100 + 2, "B", ImNodesPinShape_Circle, Pin::Input, Pin::Vec3 });

        std::string oName = "Cross_" + std::to_string(id);
        Outputpins.push_back({ id * 100 + 3, oName, ImNodesPinShape_Circle, Pin::Output, Pin::Vec3 });
    }

    void Evaluate(std::vector<Link>& links, std::vector<BaseNode*>& nodes) override
    {
        glm::vec3 a = GetConnectedValue<glm::vec3>(Inputpins[0], links, nodes);
        glm::vec3 b = GetConnectedValue<glm::vec3>(Inputpins[1], links, nodes);
        Outputpins[0].Set<glm::vec3>(glm::cross(a, b));
    }
    void DrawNodeContents() override
    {
    }
    std::string GenerateShaderCode(const std::vector<Link>& links,
        const std::vector<BaseNode*>& nodes) override
    {
        std::string a = GetConnectedVariableName(Inputpins[0], links, nodes);
        std::string b = GetConnectedVariableName(Inputpins[1], links, nodes);
        return "    vec3 " + Outputpins[0].label + " = cross(" + a + ", " + b + ");\n";
    }

    json Serialize() const override
    {
        return { {"type","CrossNode"},{"id",id},{"position",{position.x,position.y}} };
    }

    void Deserialize(const json& j) override
    {
        id = j["id"];
        position = ImVec2(j["position"][0], j["position"][1]);
        UpdatePinIds();
    }
};

struct NormalizeNode : public BaseNode
{
    NormalizeNode(int nodeId)
    {
        id = nodeId;
        title = "Normalize";
        color = ImVec4(0.4f, 0.6f, 0.9f, 1.0f);

        Inputpins.push_back({ id * 100 + 1, "Value", ImNodesPinShape_Circle, Pin::Input, Pin::Vec3 });

        std::string oName = "Norm_" + std::to_string(id);
        Outputpins.push_back({ id * 100 + 2, oName, ImNodesPinShape_Circle, Pin::Output, Pin::Vec3 });
    }

    void Evaluate(std::vector<Link>& links, std::vector<BaseNode*>& nodes) override
    {
        glm::vec3 v = GetConnectedValue<glm::vec3>(Inputpins[0], links, nodes);
        Outputpins[0].Set<glm::vec3>(glm::normalize(v));
    }
    void DrawNodeContents() override
    {
    }
    std::string GenerateShaderCode(const std::vector<Link>& links,
        const std::vector<BaseNode*>& nodes) override
    {
        std::string v = GetConnectedVariableName(Inputpins[0], links, nodes);
        return "    vec3 " + Outputpins[0].label + " = normalize(" + v + ");\n";
    }

    json Serialize() const override
    {
        return { {"type","NormalizeNode"},{"id",id},{"position",{position.x,position.y}} };
    }

    void Deserialize(const json& j) override
    {
        id = j["id"];
        position = ImVec2(j["position"][0], j["position"][1]);
        UpdatePinIds();
    }
};

struct TextureCoordsNode : public BaseNode
{
    // Node properties for UI controls
    glm::vec2 Tiling = glm::vec2(1.0f, 1.0f);
    glm::vec2 Offset = glm::vec2(0.0f, 0.0f);

    // Toggles for R, G, B pins for outputting custom values (like a ComponentMask)
    // For UVs, we mostly just expose U and V.
    bool ExposeU = false;
    bool ExposeV = false;
    bool TilingUniform = false;
    bool OffsetUniform = false;


    TextureCoordsNode(int nodeId)
    {
        id = nodeId;
        title = "TexCoord";
        color = ImVec4(0.3f, 0.7f, 0.75f, 1.0f);
        titleColor = ImVec4(0.1f, 0.4f, 0.6f, 1.0f);

        // Main Output Pin (Vec2 for UV)
        std::string outLabel = "UV_" + std::to_string(id);
        Outputpins.push_back({ id * 100 + 1, outLabel, ImNodesPinShape_Circle, Pin::Output, Pin::Vec2 });

        // Output pins for individual components
        Outputpins.push_back({ id * 100 + 2, outLabel + "_U", ImNodesPinShape_Circle, Pin::Output, Pin::Float });
        Outputpins.push_back({ id * 100 + 3, outLabel + "_V", ImNodesPinShape_Circle, Pin::Output, Pin::Float });
        // Default evaluation result (initial value for preview)
        Outputpins[0].Set<glm::vec2>(glm::vec2(0.0f, 0.0f));
        Outputpins[1].Set<float>(0.0f);
        Outputpins[2].Set<float>(0.0f);
    }

    void DrawNodeContents() override
    {
        // 1. Tiling/Scale Control
        ImGui::Text("Tiling (X/Y)");
        ImGui::SetNextItemWidth(120);
        if (ImGui::DragFloat2("##Tiling", (float*)&Tiling, 0.01f, -10.0f, 10.0f))
            Tiling = glm::max(Tiling, glm::vec2(0.01f)); // Prevent zero scaling

        ImGui::SameLine();
        ImGui::Checkbox("Uniform Tiling", &TilingUniform); // Toggle to use uniform

        // 2. Offset Control
        ImGui::Text("Offset (X/Y)");
        ImGui::SetNextItemWidth(120);
        ImGui::DragFloat2("##Offset", (float*)&Offset, 0.01f, -10.0f, 10.0f);

        ImGui::SameLine();
        ImGui::Checkbox("Uniform Offset", &OffsetUniform); // Toggle to use uniform

        // 3. Status/Toggles (if needed)
        // You can add logic here to dynamically enable/disable the U/V pins in the future.
    }

    void Evaluate(std::vector<Link>&, std::vector<BaseNode*>&) override
    {
        // In the editor, for evaluation, we just set the default UV (0,0) or (0.5, 0.5) 
        // to show a predictable preview value, as runtime UVs depend on the mesh geometry (TexCoords input).
        glm::vec2 previewUV = glm::vec2(0.5f, 0.5f) * Tiling + Offset;

        Outputpins[0].Set<glm::vec2>(previewUV);
        Outputpins[1].Set<float>(previewUV.x);
        Outputpins[2].Set<float>(previewUV.y);
    }

    std::string GenerateShaderCode(const std::vector<Link>& links,
        const std::vector<BaseNode*>& nodes) override
    {
        std::string code;
        std::string uvVar = "TexCoords"; // Built-in varying from Vertex Shader

        // 1. Uniforms (Declarations handled by Material::GenerateUniforms, but logic is here)
        // You need to ensure the uniforms are exposed to the Material system if toggled.
        // For simplicity, we hardcode the output variable based on engine inputs (TexCoords).

        std::string tileX = std::to_string(Tiling.x);
        std::string tileY = std::to_string(Tiling.y);
        std::string offsetX = std::to_string(Offset.x);
        std::string offsetY = std::to_string(Offset.y);

        // If uniforms were enabled, we'd use their names instead of hardcoded values:
        // if (TilingUniform) tileX = tileY = "TilingUniform_X";

        // 2. Main UV Calculation
        // GLSL: vec2 final_uv = TexCoords * Tiling + Offset;
        std::string finalUV = Outputpins[0].label;

        code += "    // Texture Coordinates Node " + std::to_string(id) + "\n";
        code += "    vec2 " + finalUV + " = " + uvVar + " * vec2(" + tileX + ", " + tileY + ") + vec2(" + offsetX + ", " + offsetY + ");\n";

        // 3. Component Outputs
        code += "    float " + Outputpins[1].label + " = " + finalUV + ".x;\n"; // U
        code += "    float " + Outputpins[2].label + " = " + finalUV + ".y;\n"; // V

        return code;
    }

    json Serialize() const override
    {
        return {
            {"type","TextureCoordsNode"},
            {"id",id},
            {"tiling",{Tiling.x, Tiling.y}},
            {"offset",{Offset.x, Offset.y}},
            {"position",{position.x, position.y}}
        };
    }

    void Deserialize(const json& j) override
    {
        id = j["id"];
        position = ImVec2(j["position"][0], j["position"][1]);
        if (j.contains("tiling"))
            Tiling = { j["tiling"][0], j["tiling"][1] };
        if (j.contains("offset"))
            Offset = { j["offset"][0], j["offset"][1] };
        UpdatePinIds();
    }
};