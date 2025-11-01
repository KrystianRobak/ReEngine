#pragma once

#include "UIComponent.h"
#include "imgui/imnodes.h"
#include "Graph/Node.h"
#include <MaterialSystem/Material.h>

class MaterialGraphPanel : public UIComponent
{
public:
    virtual void OnInit() override;
    // Don't need a virtual destructor here since it's an inherited class,
    // but the nodes/links are owned by the currentlySelectedMaterial when loaded.
    // If the panel owns the nodes while a material is NOT loaded, a destructor is needed.
    // However, since it only holds them while editing, and the Material class is responsible 
    // for cleanup in Save/Load, we must manually delete any nodes *not* owned by the Material
    // or ensure we correctly transfer ownership/deletion responsibility.
    // The current setup is slightly risky, but we'll manage it through LoadMaterial/SetNodes.

    virtual void Render() override;

    bool LoadMaterial(const std::string& filePath);

private:
    std::vector<BaseNode*> m_Nodes; // Nodes being edited. Owned by the Material object when loaded.
    std::vector<Link> m_Links;      // Links being edited. Owned by the Material object when loaded.
    int m_NextNodeID = 1;
    int m_NextLinkID = 1;

    std::shared_ptr<Material> currentlySelectedMaterial;
	CompiledMaterial m_CompiledMaterial;

    // Helper to generate unique pin IDs based on Node ID and a local pin index
    int GetPinID(int node_id, int pin_index) const { return node_id * 1000 + pin_index; }
};