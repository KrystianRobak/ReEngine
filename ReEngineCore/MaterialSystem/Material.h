#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "Graph/Node.h"
#include <vector>
#include <unordered_map>
#include "glm/glm.hpp"
#include "Shader.h"
#include <unordered_set>
#include "TextureData.h"
#include "Api/AssetManagerApi.h"
#include <memory> //

class CompiledMaterial
{
public:
    CompiledMaterial() = default;

    int id = 0;
    std::string name;
    std::string path;

    std::string VertexShaderCode;
    std::string FragmentShaderCode;

    // [FIX]: Use shared_ptr to prevent dangling pointers/leaks when Map resizes or Materials are copied
    std::shared_ptr<Shader> GLShader = nullptr;

    void SetId(int NewId) {
        id = NewId;
    }

    void BuildGLShader()
    {
        // shared_ptr handles deletion automatically
        GLShader = std::make_shared<Shader>(VertexShaderCode.c_str(), FragmentShaderCode.c_str(), true);
    }

    std::unordered_map<std::string, TextureResource*> textures;
    std::unordered_map<std::string, glm::vec4> m_Parameters;

    void SetTexture(const std::string& sampler, TextureResource* tex)
    {
        textures[sampler] = tex;
    }

    CompiledMaterial(std::string name, std::string path)
    {
        this->name = name;
        this->path = path;
    };

    void SetParameter(const std::string& name, const glm::vec4& value)
    {
        m_Parameters[name] = value;
    }

    glm::vec4 GetParameter(const std::string& name) const
    {
        auto it = m_Parameters.find(name);
        return (it != m_Parameters.end()) ? it->second : glm::vec4(0.0f);
    }
};

class Material
{
public:
    Material(const std::string& name = "NewMaterial")
        : name(name)
    {
    }

    ~Material()
    {
        // BUG FIX #3: m_Nodes holds raw owning pointers. Without a destructor every
        // Material destruction (or material reload) leaks every node on the heap.
        // RemoveNode() was erasing the pointer from the vector without deleting it —
        // same leak on every editor edit. Fix both here.
        for (auto* node : m_Nodes)
            delete node;
    }

    // --- Node management ---
    template<typename T, typename... Args>
    T* AddNode(Args&&... args)
    {
        T* node = new T(m_NextNodeID++);
        m_Nodes.push_back(node);
        return node;
    }

    Link& AddLink(int startPin, int endPin)
    {
        m_Links.push_back({ m_NextLinkID++, startPin, endPin });
        return m_Links.back();
    }

    void RemoveNode(int nodeId)
    {
        auto it = std::remove_if(m_Nodes.begin(), m_Nodes.end(),
            [nodeId](BaseNode* n) {
                return (n->id == nodeId);
            });
        // BUG FIX #3 cont.: delete the removed nodes before erasing so we don't leak
        for (auto jt = it; jt != m_Nodes.end(); ++jt)
            delete* jt;
        m_Nodes.erase(it, m_Nodes.end());
    }

    void SetNodes(const std::vector<BaseNode*>& nodes)
    {
        // Delete old nodes first so a graph reload doesn't leak the previous set
        for (auto* node : m_Nodes)
            delete node;
        m_Nodes = nodes;
        m_NextNodeID = 1;
        for (auto node : m_Nodes)
            m_NextNodeID = std::max(m_NextNodeID, node->id + 1);
    }

    void SetLinks(const std::vector<Link>& links)
    {
        m_Links = links;
        m_NextLinkID = 1;
        for (const auto& link : m_Links)
            m_NextLinkID = std::max(m_NextLinkID, link.id + 1);
    }

    void Evaluate()
    {
        for (auto& node : m_Nodes)
            node->Evaluate(m_Links, m_Nodes);
    }

    BaseNode* FindNodeByPin(int pinId)
    {
        for (auto* n : m_Nodes)
        {
            for (auto& in : n->Inputpins)
                if (in.id == pinId) return n;
            for (auto& out : n->Outputpins)
                if (out.id == pinId) return n;
        }
        return nullptr;
    }

    std::vector<BaseNode*> TopologicalSort()
    {
        std::vector<BaseNode*> sorted;
        std::unordered_set<BaseNode*> visited;

        std::function<void(BaseNode*)> dfs = [&](BaseNode* n)
            {
                if (visited.count(n)) return;
                visited.insert(n);
                for (auto& out : n->Outputpins)
                {
                    for (auto& link : m_Links)
                    {
                        if (link.start_pin_id == out.id)
                        {
                            BaseNode* dst = FindNodeByPin(link.end_pin_id);
                            if (dst) dfs(dst);
                        }
                    }
                }
                sorted.push_back(n);
            };

        for (auto* n : m_Nodes) dfs(n);
        std::reverse(sorted.begin(), sorted.end());
        return sorted;
    }

    CompiledMaterial Compile(AssetManagerApi* assetManager)
    {
        CompiledMaterial result(name, path);
        Evaluate();
        result.id = s_GlobalMaterialID++;
        id = result.id;
        std::string shaderBody;
        auto orderedNodes = TopologicalSort();

        for (auto* node : orderedNodes)
            shaderBody += node->GenerateShaderCode(m_Links, m_Nodes);

        for (auto* node : m_Nodes)
        {
            if (auto* texNode = dynamic_cast<TextureSampleNode*>(node))
            {
                if (texNode->texturePath.empty()) continue;
                auto texRes = assetManager->GetTexture(texNode->texturePath);
                if (texRes)
                {
                    std::string samplerName = texNode->Outputpins[0].label + "_Tex";
                    result.SetTexture(samplerName, texRes.get());
                }
            }
        }

        result.VertexShaderCode = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aWeights;
layout (location = 10) in mat4 aInstanceMatrix;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out mat3 TBN;

uniform mat4 view;
uniform mat4 projection;
const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBones[MAX_BONES];
uniform bool uIsAnimated;

void main()
{
    mat4 totalModelMatrix;
    if (uIsAnimated) 
    {
        mat4 BoneTransform = mat4(0.0);
        float totalWeight = 0.0;
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            int id = aBoneIDs[i];
            float w = aWeights[i];
            if (id < 0 || id >= MAX_BONES || w <= 0.0) continue;
            BoneTransform += finalBones[id] * w;
            totalWeight += w;
        }
        if (totalWeight == 0.0f) BoneTransform = mat4(1.0f);
        else BoneTransform = BoneTransform / totalWeight;
        totalModelMatrix = aInstanceMatrix * BoneTransform;
    }
    else
    {
        totalModelMatrix = aInstanceMatrix;
    }

    vec4 worldPos = totalModelMatrix * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    TexCoords = aTexCoords;
    mat3 normalMatrix = transpose(inverse(mat3(totalModelMatrix)));
    Normal = normalize(normalMatrix * aNormal);
    
    vec3 T = vec3(0.0);
    vec3 B = vec3(0.0);
    vec3 N = normalize(normalMatrix * aNormal);
    if (length(aTangent) > 0.001) {
        T = normalize(normalMatrix * aTangent);
        B = normalize(normalMatrix * aBitangent);
    } else {
        vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
        T = normalize(cross(up, N));
        B = cross(N, T);
    }
    TBN = mat3(T, B, N);
    gl_Position = projection * view * worldPos;
}
)";
        result.FragmentShaderCode =
            "#version 460 core\n"
            "layout (location = 0) out vec4 gPosition;\n"
            "layout (location = 1) out vec4 gNormal;\n"
            "layout (location = 2) out vec4 gAlbedoSpec;\n"
            "\n"
            "in vec2 TexCoords;\n"
            "in vec3 FragPos; \n"
            "in vec3 Normal; \n"
            "in mat3 TBN; \n"
            "\n"
            + GenerateUniforms() +
            "\n"
            "void main()\n"
            "{\n"
            + shaderBody +
            // BUG FIX #1: The OutputNode in a visual node graph only controls artist
            // data (albedo, roughness, metallic, normal map). It cannot know FragPos
            // because that is a geometry interpolant, not a material property.
            // If the node graph never writes gPosition.rgb, it stays as the GBuffer
            // clear value (0,0,0). The deferred lighting pass then reads WorldPos as
            // (0,0,0) for every material pixel, computes the wrong lightSpaceMatrix
            // transform for shadow lookups, and shadows disappear on those surfaces.
            //
            // The fix: ALWAYS overwrite gPosition.rgb with FragPos AFTER the node graph
            // body. gPosition.rgb MUST be world-space position; it is a GBuffer contract,
            // not an artist parameter. Overwriting here is correct and cannot break
            // anything the node graph legitimately produced.
            //
            // Similarly gNormal.a has no meaningful use in the lighting pass — we
            // always write 1.0 so the alpha doesn't cause silent divide-by-zero or NaN
            // in any future shader that reads that channel.
            "\n"
            "    // -- GBuffer geometry guarantee (not artist-controllable) --\n"
            "    gPosition.rgb = FragPos;\n"
            "    if (gNormal.a == 0.0) gNormal.a = 1.0;\n"
            "}\n";

        return result;
    }

    std::string GenerateUniforms()
    {
        std::string result;

        // BUG FIX #2: The old code iterated Inputpins but generated a sampler uniform
        // INSIDE that loop for TextureSampleNode. If a TextureSampleNode has N input pins
        // (e.g. UV coords, mip bias), it emitted the same "uniform sampler2D X_Tex;"
        // N times, causing a GLSL compilation error (duplicate declaration).
        // Fix: decide once per NODE whether it's a texture node or a uniform-pin node,
        // then emit at most one declaration per node.

        std::unordered_set<std::string> declaredSamplers; // guard against duplicates

        for (auto* node : m_Nodes)
        {
            if (auto* texNode = dynamic_cast<TextureSampleNode*>(node))
            {
                // One sampler per TextureSampleNode, keyed by its output label
                if (!texNode->Outputpins.empty())
                {
                    std::string samplerName = texNode->Outputpins[0].label + "_Tex";
                    if (declaredSamplers.insert(samplerName).second)
                        result += "uniform sampler2D " + samplerName + ";\n";
                }
            }
            else
            {
                // For all other nodes, emit a vec4 uniform for each pin flagged as uniform
                for (auto& pin : node->Inputpins)
                {
                    if (pin.isUniform)
                        result += "uniform vec4 " + pin.label + ";\n";
                }
            }
        }

        return result;
    }

    json Serialize() const
    {
        json j;
        j["id"] = id;
        j["name"] = name;
        j["path"] = path;
        for (auto* node : m_Nodes) j["nodes"].push_back(node->Serialize());
        for (const auto& link : m_Links)
        {
            j["links"].push_back({
                {"id", link.id},
                {"start", link.start_pin_id},
                {"end", link.end_pin_id}
                });
        }
        return j;
    }

    void Deserialize(const json& j)
    {
        id = j["id"];

        if (id >= s_GlobalMaterialID) {
            s_GlobalMaterialID = id + 1;
        }

        name = j["name"];
        path = j["path"];

        m_Nodes.clear();
        m_Links.clear();
        m_NextNodeID = 1;
        m_NextLinkID = 1;

        for (const auto& nodeData : j["nodes"])
        {
            std::string type = nodeData["type"];
            BaseNode* node = nullptr;
            if (type == "ConstantNode") node = new ConstantNode(nodeData["id"]);
            else if (type == "ConstantVec2Node") node = new ConstantVec2Node(nodeData["id"]);
            else if (type == "ConstantVec3Node") node = new ConstantVec3Node(nodeData["id"]);
            else if (type == "TextureSampleNode") node = new TextureSampleNode(nodeData["id"]);
            else if (type == "TextureCoordsNode") node = new TextureCoordsNode(nodeData["id"]);
            else if (type == "OutputNode") node = new OutputNode(nodeData["id"]);
            else if (type == "AddNode") node = new AdderNode(nodeData["id"]);
            else if (type == "MultiplyNode") node = new MultiplyNode(nodeData["id"]);
            else if (type == "DotNode") node = new DotNode(nodeData["id"]);
            else if (type == "LerpNode") node = new LerpNode(nodeData["id"]);
            else if (type == "CrossNode") node = new CrossNode(nodeData["id"]);
            else if (type == "NormalizeNode") node = new NormalizeNode(nodeData["id"]);

            if (node)
            {
                node->Deserialize(nodeData);
                m_Nodes.push_back(node);
                m_NextNodeID = std::max(m_NextNodeID, node->id + 1);
            }
        }

        for (const auto& linkData : j["links"])
        {
            Link newLink = { linkData["id"], linkData["start"], linkData["end"] };
            m_Links.push_back(newLink);
            m_NextLinkID = std::max(m_NextLinkID, newLink.id + 1);
        }
    }

    bool SaveToFile(const std::string& filepath)
    {
        for (auto* node : m_Nodes)
            ImNodes::SetNodeEditorSpacePos(node->id, node->position);
        json j = Serialize();
        std::ofstream file(filepath);
        if (!file.is_open()) return false;
        file << j.dump(4);
        return true;
    }

    bool LoadFromFile(const std::string& filepath)
    {
        path = filepath;
        std::ifstream file(filepath);
        if (!file.is_open()) return false;
        json j;
        try {
            file >> j;
            Deserialize(j);
            return true;
        }
        catch (const nlohmann::json::exception& e) {
            std::cerr << "JSON deserialization error: " << e.what() << std::endl;
            return false;
        }
    }

    std::vector<BaseNode*>& GetNodes() { return m_Nodes; }
    const std::vector<BaseNode*>& GetNodes() const { return m_Nodes; }
    std::vector<Link>& GetLinks() { return m_Links; }
    const std::vector<Link>& GetLinks() const { return m_Links; }
    const std::string& GetFilePath() const { return path; }

    static void SetNextID(int nextId) {
        if (nextId > s_GlobalMaterialID) s_GlobalMaterialID = nextId;
    }

private:
    int id;
    std::string name;
    std::string path;
    std::vector<BaseNode*> m_Nodes;
    std::vector<Link> m_Links;
    int m_NextNodeID = 1;
    int m_NextLinkID = 1;
    static inline int s_GlobalMaterialID = 1;
};