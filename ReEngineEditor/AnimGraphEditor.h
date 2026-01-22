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

	void LoadGraph(const std::string& path);
    void SaveGraph();

    void SetContext(std::shared_ptr<AnimationGraphResource> graph);

private:
    std::shared_ptr<AnimationGraphResource> currentGraph;
    std::string currentFilePath;

    bool showNodePopup = false;
    ImVec2 popupPos;


    int selectedLinkID = -1;
    int selectedNodeID = -1;
};