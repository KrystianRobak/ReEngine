#pragma once

#include <fstream>
#include <string>
#include "Graph/Node.h"
#include <vector>
#include <unordered_map>
#include "glm/glm.hpp"


class CompiledMaterial
{
public:
    int id = 0;
    std::string name;
    std::string path;

    std::string VertexShaderCode;
    std::string FragmentShaderCode;

    std::unordered_map<std::string, glm::vec4> m_Parameters;
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

    ~Material()
    {
        // This destructor is correct and handles deleting the raw BaseNode pointers.
        for (auto node : m_Nodes)
            delete node;
        m_Nodes.clear();
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
        // Find and delete the node before removing the pointer from the vector
        auto it = std::remove_if(m_Nodes.begin(), m_Nodes.end(),
            [nodeId](BaseNode* n) {
                if (n->id == nodeId) {
                    delete n; // IMPORTANT: Delete the node memory
                    return true;
                }
                return false;
            });
        m_Nodes.erase(it, m_Nodes.end());
    }

    void SetNodes(const std::vector<BaseNode*>& nodes)
    {
        // Delete the *old* nodes before replacing the vector
        for (auto node : m_Nodes)
            delete node;
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

    CompiledMaterial Compile()
    {
        CompiledMaterial result(name, path);

        Evaluate();

        std::string shaderBody;
        for (auto node : m_Nodes)
        {
            shaderBody += node->GenerateShaderCode(m_Links, m_Nodes);
        }

        // Minimal Shader Template (Vertex remains the same)
        result.VertexShaderCode = R"(
        #version 460 core
        // #version 330 core -- Removed redundancy

        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoords;

        out vec2 TexCoords;
        out vec3 FragPos;
        out vec3 Normal;

        uniform mat4 Model;
        uniform mat4 View;
        uniform mat4 Projection;
        
        void main()
        {
            FragPos = vec3(Model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(Model))) * aNormal;
            gl_Position = Projection * View * Model * vec4(aPos, 1.0);
            TexCoords = aTexCoords;
        }
        )";

        // Fragment Shader Template (Cleaned up output variable)
        result.FragmentShaderCode =
            "#version 460 core\n"
            "in vec2 TexCoords;\n"
            "in vec3 FragPos; \n"
            "in vec3 Normal; \n"
            "\n"
            + GenerateUniforms() +
            "\n"
            "out vec4 FragColor;\n" // Consistent output variable name
            "void main()\n"
            "{\n"
            + shaderBody +
            // The last line is now handled by the OutputNode's code (FragColor = vec4(finalColor, opacity);)
            // The original template had an issue here; this line is redundant/incorrect if OutputNode handles it.
            // Since OutputNode produces the final FragColor, we *remove* the redundant line.
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
                if (pin.isUniform)
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

        // Delete existing nodes before loading new ones
        for (auto node : m_Nodes)
            delete node;
        m_Nodes.clear();
        m_Links.clear();
        m_NextNodeID = 1;
        m_NextLinkID = 1;


        // --- Rebuild nodes ---
        for (const auto& nodeData : j["nodes"])
        {
            std::string type = nodeData["type"];
            BaseNode* node = nullptr;

            if (type == "ConstantNode")
                node = new ConstantNode(nodeData["id"]);
            else if (type == "AddNode")
                node = new AdderNode(nodeData["id"]);
            else if (type == "TextureSampleNode") // ADDED
                node = new TextureSampleNode(nodeData["id"]);
            else if (type == "OutputNode") // ADDED
                node = new OutputNode(nodeData["id"]);
            // Add other node types here

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

