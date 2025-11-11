#pragma once

#include "UIComponent.h"
#include "imgui/imnodes.h"
#include "Graph/Node.h"
#include "IViewport.h"
#include <MaterialSystem/Material.h>

class MaterialGraphPanel : public UIComponent
{
public:
	MaterialGraphPanel() = default;

    virtual void OnInit() override;

    virtual void Render() override;

    bool LoadMaterial(const std::string& filePath);

private:
    std::vector<BaseNode*> m_Nodes; // Nodes being edited. Owned by the Material object when loaded.
    std::vector<Link> m_Links;      // Links being edited. Owned by the Material object when loaded.
    int m_NextNodeID = 1;
    int m_NextLinkID = 1;

    IViewport* viewport = nullptr;

    std::shared_ptr<Material> currentlySelectedMaterial;
	CompiledMaterial* m_CompiledMaterial;

    // Helper to generate unique pin IDs based on Node ID and a local pin index
    int GetPinID(int node_id, int pin_index) const { return node_id * 1000 + pin_index; }
};