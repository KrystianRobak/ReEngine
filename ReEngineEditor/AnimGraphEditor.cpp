#include "AnimGraphEditor.h"

AnimGraphEditor::AnimGraphEditor() {
    // Note: ImNodes Context creation should happen once ideally, 
    // but putting it here for safety if this is a singleton-ish window.
}

AnimGraphEditor::~AnimGraphEditor() {
    ImNodes::DestroyContext();
}

void AnimGraphEditor::OnInit() {
    ImNodes::CreateContext();
    // Optional: Set Style
    ImNodes::StyleColorsDark();
}

void AnimGraphEditor::SetContext(std::shared_ptr<AnimationGraphResource> graph) {
    currentGraph = graph;
}

void AnimGraphEditor::Render()
{
    // 1. Window Management (Inherited from UIComponent)
    if (!ImGui::Begin("Animation Graph Editor", &closed)) {
        ImGui::End();
        return;
    }

    if (!RenderWindowTopBar()) { ImGui::End(); return; }

    if (!currentGraph) {
        ImGui::Text("No Animation Graph Selected.");
        if (ImGui::Button("Create New Graph")) {
            currentGraph = std::make_shared<AnimationGraphResource>();
            currentGraph->EntryNodeID = -1;
        }
        ImGui::End();
        return;
    }

    // --- LEFT PANEL: VARIABLES ---
    ImGui::BeginChild("Variables", ImVec2(200, 0), true);
    ImGui::Text("Parameters");
    ImGui::Separator();

    static char newVarName[64] = "";
    ImGui::InputText("Name", newVarName, 64);
    if (ImGui::Button("Add Float")) {
        currentGraph->DefaultBlackboard[newVarName] = AnimVar(0.0f);
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Bool")) {
        currentGraph->DefaultBlackboard[newVarName] = AnimVar(false);
    }

    ImGui::Separator();
    for (auto& [name, var] : currentGraph->DefaultBlackboard) {
        ImGui::Text("%s", name.c_str());
        ImGui::SameLine();
        if (var.Type == AnimVarType::Float) ImGui::TextColored(ImVec4(0, 1, 0, 1), "[Float]");
        else ImGui::TextColored(ImVec4(1, 0, 0, 1), "[Bool]");
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // --- RIGHT PANEL: GRAPH ---
    ImGui::BeginChild("GraphArea", ImVec2(0, 0), true);

    ImNodes::BeginNodeEditor();

    // 2. DRAW NODES
    for (auto& node : currentGraph->Nodes) {
        ImNodes::BeginNode(node.ID);

        ImNodes::BeginNodeTitleBar();
        ImGui::Text("%s", node.AnimationName.c_str());
        ImNodes::EndNodeTitleBar();

        // Input Pin
        ImNodes::BeginInputAttribute(node.ID << 8 | 0);
        ImGui::Text("In");
        ImNodes::EndInputAttribute();

        // Node Content
        ImGui::PushItemWidth(100);
        if (node.ID == currentGraph->EntryNodeID) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "ENTRY");
        }
        ImGui::PopItemWidth();

        // Output Pin
        ImNodes::BeginOutputAttribute(node.ID << 8 | 1);
        ImGui::Text("Out");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    // 3. DRAW LINKS
    for (const auto& link : currentGraph->Transitions) {
        // ID construction: Pack generic ID
        ImNodes::Link(link.ID, link.FromNodeID << 8 | 1, link.ToNodeID << 8 | 0);
    }

    ImNodes::EndNodeEditor();

    // 4. HANDLE INTERACTION

    // Check for Link Creation
    int startAttr, endAttr;
    if (ImNodes::IsLinkCreated(&startAttr, &endAttr)) {
        int startNode = startAttr >> 8;
        int endNode = endAttr >> 8;

        GraphTransition newTrans;
        newTrans.ID = ++currentGraph->NextLinkID;
        newTrans.FromNodeID = startNode;
        newTrans.ToNodeID = endNode;
        newTrans.ConditionParam = ""; // Default empty
        newTrans.Threshold = 0.0f;

        currentGraph->Transitions.push_back(newTrans);
    }

    // Check for Right Click (Context Menu)
    if (ImGui::IsMouseClicked(1) && ImNodes::IsEditorHovered()) {
        ImGui::OpenPopup("NodeGraphContext");
        popupPos = ImGui::GetMousePos();
    }

    if (ImGui::BeginPopup("NodeGraphContext")) {
        if (ImGui::MenuItem("Add Animation Node")) {
            GraphNode newNode;
            newNode.ID = ++currentGraph->NextNodeID;
            newNode.AnimationName = "New_Anim";
            // ImNodes does not support SetNodePos directly in Immediate mode easily 
            // without a context helper, but you can use ImNodes::SetNodeScreenSpacePos
            ImNodes::SetNodeScreenSpacePos(newNode.ID, popupPos);

            if (currentGraph->EntryNodeID == -1) currentGraph->EntryNodeID = newNode.ID;

            currentGraph->Nodes.push_back(newNode);
        }
        ImGui::EndPopup();
    }

    // Check Selection for Properties Panel (Transition logic editing)
    int numSelectedLinks = ImNodes::NumSelectedLinks();
    if (numSelectedLinks > 0) {
        std::vector<int> selectedLinks; selectedLinks.resize(numSelectedLinks);
        ImNodes::GetSelectedLinks(selectedLinks.data());
        int linkID = selectedLinks[0];

        // Find link and show editor popup/overlay
        for (auto& link : currentGraph->Transitions) {
            if (link.ID == linkID) {
                ImGui::BeginTooltip();
                ImGui::Text("Transition Logic");
                // In a real engine, use a Combo box populated from currentGraph->DefaultBlackboard keys
                static char buf[64]; strcpy(buf, link.ConditionParam.c_str());
                if (ImGui::InputText("Param", buf, 64)) link.ConditionParam = std::string(buf);

                ImGui::DragFloat("Threshold", &link.Threshold);
                // Enum Combo for Operation (>, <, =)
                const char* items[] = { ">", "<", "=", "!=" };
                int item_current = (int)link.Operation;
                if (ImGui::Combo("Op", &item_current, items, 4)) link.Operation = (ConditionOp)item_current;

                ImGui::EndTooltip();
                break;
            }
        }
    }

    ImGui::EndChild(); // GraphArea
    ImGui::End(); // Main Window
}