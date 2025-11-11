#include "MaterialGraphPanel.h"
#include <algorithm>
#include <iostream> // For error checking

// --- Local Data Structures ---

// --------------------------------------------------------

void MaterialGraphPanel::OnInit()
{
    viewport = engineApp->CreateNewViewport("MaterialGraphViewport");
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

    // static bool p_open = true; // Unused, removed

    ImGui::Begin("Material Graph", nullptr, ImGuiWindowFlags_MenuBar);

    if (!RenderWindowTopBar())
    {
        ImGui::End();
        return;
    }

    // Menu bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::Button("Apply"))
        {
            if (currentlySelectedMaterial)
            {
                // Update material object with current graph state
                currentlySelectedMaterial->SetNodes(m_Nodes);
                currentlySelectedMaterial->SetLinks(m_Links);

                // Save and Compile
                currentlySelectedMaterial->SaveToFile(currentlySelectedMaterial->GetFilePath());
                CompiledMaterial mat = currentlySelectedMaterial->Compile(); // Compile saves the result inside the Material object

                m_CompiledMaterial = &mat;

                ImGui::Text("Compiled and Applied!");
            }
            else
            {
                ImGui::Text("Error: No material selected to apply.");
            }
        }
        if (ImGui::Button("Save As"))
        {
            if (currentlySelectedMaterial)
            {
                // Note: "Save As" usually prompts for a *new* path, but for now we'll save to the current path.
                // If you implement a file dialog, update GetFilePath().
                currentlySelectedMaterial->SetNodes(m_Nodes);
                currentlySelectedMaterial->SetLinks(m_Links);
                currentlySelectedMaterial->SaveToFile(currentlySelectedMaterial->GetFilePath());
                ImGui::Text("Saved!");
            }
            else
            {
                ImGui::Text("Error: No material selected to save.");
            }
        }
        ImGui::EndMenuBar();
    }


   

    ImNodes::BeginNodeEditor();

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) // Check if editor is hovered
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
        if (ImGui::MenuItem("Texture Sample Node"))
        {
            m_Nodes.push_back(CreateNode<TextureSampleNode>(m_NextNodeID, clickPos));
        }
        if (ImGui::MenuItem("Output Node"))
        {
            // OutputNode is typically unique and its position is often fixed/special
            // For now, allow multiple, but a real system would only allow one.
            m_Nodes.push_back(CreateNode<OutputNode>(m_NextNodeID, clickPos));
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Clear All"))
        {
            // The nodes in m_Nodes are raw pointers. They must be deleted!
            // When a material is loaded, its nodes are assigned to m_Nodes.
            // We should ensure the Material destructor is responsible for deletion.
            // For safety, clear all, and reset the material's internal lists.
            for (BaseNode* node : m_Nodes)
                delete node; // IMPORTANT: Delete the raw pointers before clearing the vector

            m_Nodes.clear();
            m_Links.clear();
            m_NextNodeID = 1;
            m_NextLinkID = 1;

            if (currentlySelectedMaterial)
            {
                currentlySelectedMaterial->SetNodes(m_Nodes); // Update the material (now empty)
                currentlySelectedMaterial->SetLinks(m_Links);
            }
        }

        ImGui::EndPopup();
    }

    // Draw existing nodes
    for (auto& node : m_Nodes)
        node->DrawNode();

    // Draw links
    for (auto& link : m_Links)
        ImNodes::Link(link.id, link.start_pin_id, link.end_pin_id);

    

    ImNodes::EndNodeEditor();



    // Check for node deletion
    const int numSelectedNodes = ImNodes::NumSelectedNodes();
    if (numSelectedNodes > 0 && ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        static std::vector<int> selectedNodes;
        selectedNodes.resize(numSelectedNodes);
        ImNodes::GetSelectedNodes(selectedNodes.data());

        // Delete nodes and associated links
        for (int nodeId : selectedNodes)
        {
            // Find and delete the node pointer
            auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(),
                [nodeId](BaseNode* n) { return n->id == nodeId; });
            if (it != m_Nodes.end())
            {
                delete* it; // Free memory
                m_Nodes.erase(it);
            }

            // Remove all links connected to this node's pins
            m_Links.erase(std::remove_if(m_Links.begin(), m_Links.end(),
                [nodeId](const Link& link) {
                    // Check if either pin belongs to the node
                    return (link.start_pin_id / 100) == nodeId || (link.end_pin_id / 100) == nodeId;
                }), m_Links.end());
        }
    }

    // check for link deletion
	const int numSelectedLinks = ImNodes::NumSelectedLinks();
	if (numSelectedLinks > 0 && ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		static std::vector<int> selectedLinks;
		selectedLinks.resize(numSelectedLinks);
		ImNodes::GetSelectedLinks(selectedLinks.data());
		for (int linkId : selectedLinks)
		{
			m_Links.erase(std::remove_if(m_Links.begin(), m_Links.end(),
                [linkId](const Link& link) {
                    return link.id == linkId;
				}), m_Links.end());
		}
	}


    // --- Check for new links safely ---
    int startPinId = 0;
    int endPinId = 0;
    if (ImNodes::IsLinkCreated(&startPinId, &endPinId))
    {
        //if (startPinId != 0 && endPinId != 0)
        //{
        //    // Basic sanity check to prevent self-linking or output-output/input-input
        //    // A proper implementation would check Pin::Type and Pin::DataType compatibility
        //    bool isStartInput = (startPinId % 1000) < 100; // Heuristic: Input pins < 100, Output pins >= 100
        //    bool isEndInput = (endPinId % 1000) < 100;

        //    // Check for connecting Input to Output or vice versa
        //    if (isStartInput != isEndInput)
        //    {
        //        // Ensure link direction is always from Output (start) to Input (end)
        //        if (isStartInput)
        //        {
        //            // Swap to enforce Output -> Input
        //            std::swap(startPinId, endPinId);
        //        }

        //        // Check for duplicate links (simple check: same pin pair)
        //        bool duplicate = false;
        //        for (const auto& link : m_Links)
        //        {
        //            if (link.start_pin_id == startPinId && link.end_pin_id == endPinId)
        //            {
        //                duplicate = true;
        //                break;
        //            }
        //        }

        //        if (!duplicate)
        //        {
        //            // Check for single-input attributes (i.e. if endPinId is already linked)
        //            bool alreadyLinked = false;
        //            for (const auto& link : m_Links)
        //            {
        //                if (link.end_pin_id == endPinId)
        //                {
        //                    alreadyLinked = true;
        //                    break;
        //                }
        //            }

        //            if (!alreadyLinked)
        //            {
                        m_Links.push_back({ m_NextLinkID++, startPinId, endPinId });
  /*                  }
                }
            }
        }*/
    }

    // Check for link destruction
    int linkId = 0;
    if (ImNodes::IsLinkDestroyed(&linkId))
    {
        m_Links.erase(std::remove_if(m_Links.begin(), m_Links.end(),
            [linkId](const Link& link) {
                return link.id == linkId;
            }), m_Links.end());
    }


    // --- Runtime evaluation ---
    // Note: In a real system, you'd only evaluate if the graph changed.
    for (auto& node : m_Nodes)
        node->Evaluate(m_Links, m_Nodes);

    ImGui::End();
}

bool MaterialGraphPanel::LoadMaterial(const std::string& filePath)
{
    // 1. Initial Load: If no material is currently selected
    if (currentlySelectedMaterial == nullptr)
    {
        // Load the material from file
        Material mat;
        if (!mat.LoadFromFile(filePath))
        {
            std::cerr << "Error: Failed to load material from " << filePath << std::endl;
            return false;
        }

        // Steal the nodes and links from the temporary 'mat' object.
        // The temporary 'mat' object's destructor will run and clean up its now-empty vectors, 
        // preventing double deletion and memory leak issues.
        m_Nodes.swap(mat.GetNodes());
        m_Links.swap(mat.GetLinks());

        m_NextNodeID = m_Nodes.size() + 1;
		m_NextLinkID = m_Links.size() + 1;


        // Create the shared pointer from the loaded material object
        currentlySelectedMaterial = std::make_shared<Material>(mat);

        // Set node positions
        for (auto* node : m_Nodes)
            ImNodes::SetNodeEditorSpacePos(node->id, node->position);

        return true;
    }

    // 2. Load New Material: If a different material is selected
    if (currentlySelectedMaterial->GetFilePath() != filePath)
    {
        // A. Save/Cleanup the currently edited material
        currentlySelectedMaterial->SetNodes(m_Nodes);
        currentlySelectedMaterial->SetLinks(m_Links);
        currentlySelectedMaterial->SaveToFile(currentlySelectedMaterial->GetFilePath());

        // m_Nodes now points to the Material's internal node pointers. We need to clear 
        // the *panel's* raw pointers without deleting the memory, since the Material owns 
        // and manages the memory now. We'll let the next load step replace them.
        m_Nodes.clear();
        m_Links.clear();

        // B. Load the new material
        Material newMat;
        if (!newMat.LoadFromFile(filePath))
        {
            std::cerr << "Error: Failed to load material from " << filePath << std::endl;
            return false;
        }

        // Steal the new nodes and links
        m_Nodes.swap(newMat.GetNodes());
        m_Links.swap(newMat.GetLinks());

        // Update the currently selected material shared pointer
        currentlySelectedMaterial = std::make_shared<Material>(newMat);

        // Set node positions
        for (auto* node : m_Nodes)
            ImNodes::SetNodeEditorSpacePos(node->id, node->position);

        return true;
    }

    // 3. Material is already loaded and is the same one
    return false;
}