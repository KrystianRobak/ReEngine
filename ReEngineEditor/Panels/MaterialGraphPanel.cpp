#include "MaterialGraphPanel.h"
#include <algorithm>

// --- Local Data Structures ---



// --------------------------------------------------------

void MaterialGraphPanel::OnInit()
{
    // Initialize ImNodes context
    ImNodes::CreateContext();

}

template<typename T>
T* CreateNode(int& nextNodeID, const ImVec2& position)
{
    T* node = new T(nextNodeID++);
    node->position = position;
    ImNodes::SetNodeEditorSpacePos(node->id, position);
    return node;
}

void MaterialGraphPanel::Render()
{

    static bool p_open = true;

    ImGui::Begin("Material Graph", nullptr, ImGuiWindowFlags_MenuBar);

    // Menu bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::Button("Apply"))
        {
			currentlySelectedMaterial->SetNodes(m_Nodes);
			currentlySelectedMaterial->SetLinks(m_Links);
			currentlySelectedMaterial->SaveToFile(currentlySelectedMaterial->GetFilePath());

            CompiledMaterial mat = currentlySelectedMaterial->Compile();

            ImGui::Text("Compiled!");
        }
        if (ImGui::Button("Save As"))
        {
			currentlySelectedMaterial->SetNodes(m_Nodes);
			currentlySelectedMaterial->SetLinks(m_Links);
			currentlySelectedMaterial->SaveToFile(currentlySelectedMaterial->GetFilePath());
			ImGui::Text("Saved!");
        }
        ImGui::EndMenuBar();
    }

    ImNodes::BeginNodeEditor();

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("NodeContextMenu");
    }

    if (ImGui::BeginPopup("NodeContextMenu"))
    {
        ImVec2 clickPos = ImGui::GetMousePosOnOpeningCurrentPopup();

        if (ImGui::MenuItem("Constant Node"))
        {
			m_Nodes.push_back(CreateNode<ConstantNode>(m_NextNodeID, clickPos));
        }
        if (ImGui::MenuItem("Add Node"))
        {
			m_Nodes.push_back(CreateNode<AdderNode>(m_NextNodeID, clickPos));
        }
        if(ImGui::MenuItem("Texture Sample Node"))
        {
			m_Nodes.push_back(CreateNode<TextureSampleNode>(m_NextNodeID, clickPos));
		}
        if (ImGui::MenuItem("Output Node"))
        {
			m_Nodes.push_back(CreateNode<OutputNode>(m_NextNodeID, clickPos));
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Clear All"))
        {
            m_Nodes.clear();
            m_Links.clear();
        }

        ImGui::EndPopup();
    }

    // Draw existing nodes
    for (auto& node : m_Nodes)
        node->DrawNode();

    // Draw links
    for (auto& link : m_Links)
        ImNodes::Link(link.id, link.start_pin_id, link.end_pin_id);

    //
    // ?? Right-click context menu for adding new nodes
    //

    ImNodes::EndNodeEditor();

    // --- Check for new links safely ---
    int startPinId = 0;
    int endPinId = 0;
    if (ImNodes::IsLinkCreated(&startPinId, &endPinId))
    {
        if (startPinId != 0 && endPinId != 0)
            m_Links.push_back({ m_NextLinkID++, startPinId, endPinId });
    }

    // --- Runtime evaluation ---
    for (auto& node : m_Nodes)
        node->Evaluate(m_Links, m_Nodes);

    ImGui::End();
}

bool MaterialGraphPanel::LoadMaterial(const std::string& filePath)
{
    if (currentlySelectedMaterial == nullptr)
    {
        Material mat;
        mat.LoadFromFile(filePath);

		m_Nodes.swap(mat.GetNodes());
		m_Links.swap(mat.GetLinks());

		currentlySelectedMaterial = std::make_shared<Material>(mat);

        for (auto* node : m_Nodes)
            ImNodes::SetNodeEditorSpacePos(node->id, node->position);
    }
    if(currentlySelectedMaterial->GetFilePath() == filePath)
		return false;
    else
    {
		currentlySelectedMaterial->SetNodes(m_Nodes);
		currentlySelectedMaterial->SetLinks(m_Links);

		currentlySelectedMaterial->SaveToFile(currentlySelectedMaterial->GetFilePath());

        Material mat;
        mat.LoadFromFile(filePath);

        m_Nodes.emplace_back(mat.GetNodes());
        m_Links.emplace_back(mat.GetLinks());


		std::shared_ptr<Material> sharedMat = std::make_shared<Material>(mat);
        currentlySelectedMaterial.swap(sharedMat);

        for (auto* node : m_Nodes)
            ImNodes::SetNodeEditorSpacePos(node->id, node->position);
    }
}

