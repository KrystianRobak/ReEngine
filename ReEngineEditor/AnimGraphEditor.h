#pragma once
#include "UIComponent.h"
#include <imgui/imnodes.h>
#include <AnimGraph.h>
class AnimGraphEditor : public UIComponent
{
public:
    AnimGraphEditor();
    ~AnimGraphEditor();

    void OnInit() override;
    void Render() override;

    // Call this to open a specific graph for editing
    void SetContext(std::shared_ptr<AnimationGraphResource> graph);

private:
    std::shared_ptr<AnimationGraphResource> currentGraph;

    // UI State
    bool showNodePopup = false;
    ImVec2 popupPos;

    // Temporary Link State
    int selectedLinkID = -1;
    int selectedNodeID = -1;
};