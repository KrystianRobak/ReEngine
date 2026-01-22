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

class CompiledMaterial
{
public:
	CompiledMaterial() = default;

    int id = 0;
    std::string name;
    std::string path;

    std::string VertexShaderCode;
    std::string FragmentShaderCode;
    
    Shader* GLShader = nullptr;
    
    void SetId(int NewId) {
		id = NewId;
    }

    void BuildGLShader()
    {
        if (GLShader)
            delete GLShader;

        GLShader = new Shader(VertexShaderCode.c_str(), FragmentShaderCode.c_str(), true);
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

private:

};

class Material
{
public:
    Material(const std::string& name = "NewMaterial")
        : name(name)
    {
        id = s_GlobalMaterialID++;
    }

    ~Material() = default;

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
        // Find and delete the node before removing the pointer from the vector
        auto it = std::remove_if(m_Nodes.begin(), m_Nodes.end(),
            [nodeId](BaseNode* n) {
                return (n->id == nodeId);
            });
        m_Nodes.erase(it, m_Nodes.end());
    }

    void SetNodes(const std::vector<BaseNode*>& nodes)
    {
        m_Nodes = nodes;

        // Recalculate next ID after setting the nodes
        m_NextNodeID = 1;
        for (auto node : m_Nodes)
            m_NextNodeID = std::max(m_NextNodeID, node->id + 1);
    }

    void SetLinks(const std::vector<Link>& links)
    {
        m_Links = links;
        // Recalculate next ID after setting the links
        m_NextLinkID = 1;
        for (const auto& link : m_Links)
            m_NextLinkID = std::max(m_NextLinkID, link.id + 1);
    }

    void Evaluate()
    {
        // In a real graph system, you’d topologically sort nodes here.
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

                // find outgoing links
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

        for (auto* n : m_Nodes)
            dfs(n);

        std::reverse(sorted.begin(), sorted.end());

        return sorted;
    }

    CompiledMaterial Compile(AssetManagerApi* assetManager)
    {
        CompiledMaterial result(name, path);

        Evaluate();

        std::string shaderBody;

        auto orderedNodes = TopologicalSort();

        for (auto* node : orderedNodes)
            shaderBody += node->GenerateShaderCode(m_Links, m_Nodes);

        for (auto* node : m_Nodes)
        {
            // Check if this node is a TextureSampleNode
            if (auto* texNode = dynamic_cast<TextureSampleNode*>(node))
            {
                if (texNode->texturePath.empty()) continue;

                // Ask AssetManager for the GPU resource
                auto texRes = assetManager->GetTexture(texNode->texturePath);

                if (texRes)
                {
                    // MUST match the name generated in TextureSampleNode::GenerateShaderCode
                    // logic: Outputpins[0].label + "_Tex"
                    std::string samplerName = texNode->Outputpins[0].label + "_Tex";

                    // Store it in the compiled material
                    result.SetTexture(samplerName, texRes.get());
                }
            }
        }

        // --- UPDATED VERTEX SHADER (Matches GBuffer.vs logic) ---
        // Includes BoneIDs (loc 5), Weights (loc 6), and Animation Logic
        result.VertexShaderCode = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// Bone Data
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aWeights;

// Instance Matrix
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

            if (id < 0 || id >= MAX_BONES || w <= 0.0)
                continue;
            
            BoneTransform += finalBones[id] * w;
            totalWeight += w;
        }

        // Safety: If no valid weights, use identity to prevent mesh disappearing
        if (totalWeight == 0.0f) BoneTransform = mat4(1.0f);
        else BoneTransform = BoneTransform / totalWeight; // Normalize if needed

        // Combine: Instance * Bone
        totalModelMatrix = aInstanceMatrix * BoneTransform;
    }
    else
    {
        totalModelMatrix = aInstanceMatrix;
    }

    vec4 worldPos = totalModelMatrix * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    TexCoords = aTexCoords;
    
    // Normal Matrix
    // Note: In production, pass a NormalMatrix attribute to avoid inverse() here
    mat3 normalMatrix = transpose(inverse(mat3(totalModelMatrix)));
    Normal = normalize(normalMatrix * aNormal);
    
    // Calculate TBN
    vec3 T = vec3(0.0);
    vec3 B = vec3(0.0);
    vec3 N = normalize(normalMatrix * aNormal);

    if (length(aTangent) > 0.001) {
        T = normalize(normalMatrix * aTangent);
        B = normalize(normalMatrix * aBitangent);
    } else {
        // Fallback tangent generation
        vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
        T = normalize(cross(up, N));
        B = cross(N, T);
    }
    
    TBN = mat3(T, B, N);
    
    gl_Position = projection * view * worldPos;
}
)";

        // --- FRAGMENT SHADER TEMPLATE ---
        // Matches InitGBuffer attachments  and Node.h outputs
        result.FragmentShaderCode =
            "#version 460 core\n"
            "layout (location = 0) out vec4 gPosition;\n"   // RGB=Pos, A=Metallic
            "layout (location = 1) out vec4 gNormal;\n"     // RGB=Normal, A=Unused
            "layout (location = 2) out vec4 gAlbedoSpec;\n" // RGB=Albedo, A=Roughness
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
            "}\n";

        return result;
    }

    std::string GenerateUniforms()
    {
        std::string result;

        for (auto node : m_Nodes)
        {
            for (auto& pin : node->Inputpins)
            {
                if (auto* texNode = dynamic_cast<TextureSampleNode*>(node))
                {
                    std::string samplerName = texNode->Outputpins[0].label + "_Tex";
                    result += "uniform sampler2D " + samplerName + ";\n";
                }
                else if (pin.isUniform)
                    result += "uniform vec4 " + pin.label + ";\n";
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

        // Serialize nodes
        for (auto* node : m_Nodes)
            j["nodes"].push_back(node->Serialize());

        // Serialize links
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
        name = j["name"];
        path = j["path"];

        // Clear existing data
        m_Nodes.clear();
        m_Links.clear();
        m_NextNodeID = 1;
        m_NextLinkID = 1;

        // --- Rebuild nodes ---
        for (const auto& nodeData : j["nodes"])
        {
            std::string type = nodeData["type"];
            BaseNode* node = nullptr;

            // Basic & Material Nodes
            if (type == "ConstantNode")
                node = new ConstantNode(nodeData["id"]);
            else if (type == "ConstantVec2Node")
                node = new ConstantVec2Node(nodeData["id"]);
            else if (type == "ConstantVec3Node")
                node = new ConstantVec3Node(nodeData["id"]);
            else if (type == "TextureSampleNode")
                node = new TextureSampleNode(nodeData["id"]);
            else if (type == "TextureCoordsNode")
                node = new TextureCoordsNode(nodeData["id"]);
            else if (type == "OutputNode")
                node = new OutputNode(nodeData["id"]);

            // Math Nodes
            else if (type == "AddNode")
                node = new AdderNode(nodeData["id"]); // Note: Checks 'AdderNode' struct in Node.h
            else if (type == "MultiplyNode")
                node = new MultiplyNode(nodeData["id"]);
            else if (type == "DotNode")
                node = new DotNode(nodeData["id"]);
            else if (type == "LerpNode")
                node = new LerpNode(nodeData["id"]);
            else if (type == "CrossNode")
                node = new CrossNode(nodeData["id"]);
            else if (type == "NormalizeNode")
                node = new NormalizeNode(nodeData["id"]);

            if (node)
            {
                node->Deserialize(nodeData);
                m_Nodes.push_back(node);
                m_NextNodeID = std::max(m_NextNodeID, node->id + 1);
            }
        }

        // --- Rebuild links ---
        for (const auto& linkData : j["links"])
        {
            Link newLink = {
                linkData["id"],
                linkData["start"],
                linkData["end"]
            };
            m_Links.push_back(newLink);
            m_NextLinkID = std::max(m_NextLinkID, newLink.id + 1);
        }
    }

    bool SaveToFile(const std::string& filepath)
    {
        // Capture latest positions before saving
        for (auto* node : m_Nodes)
            ImNodes::SetNodeEditorSpacePos(node->id, node->position); // Use the current position

        json j = Serialize();
        std::ofstream file(filepath);
        if (!file.is_open()) return false;
        file << j.dump(4);
        return true;
    }


    bool LoadFromFile(const std::string& filepath)
    {
        path = filepath; // Set the path before deserializing
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


    // --- Getters ---
    // The const version returns a const reference, but we need to return a non-const 
    // reference to allow the MaterialGraphPanel to 'swap' the contents out safely in LoadMaterial.
    std::vector<BaseNode*>& GetNodes() { return m_Nodes; }
    const std::vector<BaseNode*>& GetNodes() const { return m_Nodes; }

    std::vector<Link>& GetLinks() { return m_Links; }
    const std::vector<Link>& GetLinks() const { return m_Links; }

    const std::string& GetFilePath() const { return path; }

private:
    int id;
    std::string name;
    std::string path;

    std::vector<BaseNode*> m_Nodes; // Owned memory: raw pointers are deleted in dtor/SetNodes
    std::vector<Link> m_Links;
    int m_NextNodeID = 1;
    int m_NextLinkID = 1;

    static inline int s_GlobalMaterialID = 1;
};

