#pragma once

#include "UIComponent.h"
#include "imgui/imnodes.h"
#include "Graph/Node.h"

class MaterialGraphPanel : public UIComponent
{
public:
    virtual void OnInit() override;
    virtual void Render() override;

private:
    std::vector<BaseNode*> m_Nodes;
    std::vector<Link> m_Links;
    int m_NextNodeID = 1;
    int m_NextLinkID = 1;

    // Helper to generate unique pin IDs based on Node ID and a local pin index
    int GetPinID(int node_id, int pin_index) const { return node_id * 1000 + pin_index; }

    
};
