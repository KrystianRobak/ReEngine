#include "AnimGraphEditor.h"
#include <Logger.h>
#include <fstream>
#include <json/json.hpp>
#include <filesystem>

using json = nlohmann::json;

AnimGraphEditor::AnimGraphEditor() {
}

AnimGraphEditor::~AnimGraphEditor() {
    ImNodes::DestroyContext();
}

void AnimGraphEditor::OnInit() {
    ImNodes::CreateContext();
    ImNodes::StyleColorsDark();

    ImNodes::GetIO().LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;
}

void AnimGraphEditor::SetContext(std::shared_ptr<AnimationGraphResource> graph) {
    currentGraph = graph;
}

void AnimGraphEditor::Render()
{

    if (!ImGui::Begin("Animation Graph Editor", &closed)) {
        ImGui::End();
        return;
    }

    if (!RenderWindowTopBar()) {
        if (pendingRemove && currentGraph && !currentFilePath.empty()) {
            SaveGraph();
        }
        ImGui::End();
        return;
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save", "Ctrl+S")) { SaveGraph(); }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (!currentGraph) {
        ImGui::Text("No Animation Graph Selected.");
        if (ImGui::Button("Create New Graph")) {
            currentGraph = std::make_shared<AnimationGraphResource>();
            currentGraph->EntryNodeID = -1;
        }
        ImGui::End();
        return;
    }


    ImGui::BeginChild("LeftPanel", ImVec2(300, 0), true);

    if (ImGui::CollapsingHeader("Blackboard / Variables", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Spacing();
        static char newVarName[64] = "";
        ImGui::InputTextWithHint("##NewVar", "New Param Name", newVarName, 64);

        if (ImGui::Button("Add Float")) {
            if (strlen(newVarName) > 0) currentGraph->DefaultBlackboard[newVarName] = AnimVar(0.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Bool")) {
            if (strlen(newVarName) > 0) currentGraph->DefaultBlackboard[newVarName] = AnimVar(false);
        }

        ImGui::Separator();

        std::vector<std::string> varsToDelete;
        for (auto& [name, var] : currentGraph->DefaultBlackboard) {
            ImGui::PushID(name.c_str());

            if (ImGui::Button("X")) { varsToDelete.push_back(name); }
            ImGui::SameLine();
            ImGui::Text("%s", name.c_str());
            ImGui::SameLine();

            if (var.Type == AnimVarType::Float) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "[Float]");
            }
            else {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "[Bool]");
            }
            ImGui::PopID();
        }

        for (const auto& key : varsToDelete) currentGraph->DefaultBlackboard.erase(key);
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Inspector", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Spacing();

        if (selectedNodeID != -1)
        {

            GraphNode* nodePtr = nullptr;
            for (auto& n : currentGraph->Nodes) {
                if (n.ID == selectedNodeID) { nodePtr = &n; break; }
            }

            if (nodePtr)
            {
                ImGui::TextDisabled("State Properties");

                char buffer[128];
                memset(buffer, 0, sizeof(buffer));
                strncpy_s(buffer, nodePtr->Name.c_str(), sizeof(buffer) - 1);
                if (ImGui::InputText("State Name", buffer, sizeof(buffer))) {
                    nodePtr->Name = std::string(buffer);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                DrawAssetSlot("Animation", nodePtr->AnimationPath, "ASSET_ANIMATION", FileType::Animation);

                ImGui::Spacing();

                if (nodePtr->ID == currentGraph->EntryNodeID) {
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "This is the ENTRY node.");
                }
                else {
                    if (ImGui::Button("Set as Entry Node")) {
                        currentGraph->EntryNodeID = nodePtr->ID;
                    }
                }
            }
        }
        else if (selectedLinkID != -1)
        {
            GraphTransition* linkPtr = nullptr;
            for (auto& l : currentGraph->Transitions) {
                if (l.ID == selectedLinkID) { linkPtr = &l; break; }
            }

            if (linkPtr)
            {
                ImGui::TextDisabled("Transition Logic");
                ImGui::Separator();

                char buf[64];
                memset(buf, 0, sizeof(buf));
                strncpy_s(buf, linkPtr->ConditionParam.c_str(), sizeof(buf) - 1);
                if (ImGui::InputText("Parameter", buf, 64)) {
                    linkPtr->ConditionParam = std::string(buf);
                }

                ImGui::DragFloat("Threshold", &linkPtr->Threshold, 0.1f);

                const char* items[] = { "GreaterThan (>)", "LessThan (<)", "Equals (=)", "NotEquals (!=)" };
                int item_current = (int)linkPtr->Operation;
                if (ImGui::Combo("Condition", &item_current, items, 4)) {
                    linkPtr->Operation = (ConditionOp)item_current;
                }

                ImGui::Spacing();
                ImGui::TextWrapped("Transition will trigger when '%s' %s %.2f",
                    linkPtr->ConditionParam.empty() ? "?" : linkPtr->ConditionParam.c_str(),
                    item_current == 0 ? ">" : (item_current == 1 ? "<" : (item_current == 2 ? "==" : "!=")),
                    linkPtr->Threshold
                );
            }
        }
        else
        {
            ImGui::TextDisabled("Select a Node or Link to edit.");
        }
    }

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("GraphArea", ImVec2(0, 0), true);

    ImNodes::BeginNodeEditor();


    for (auto& node : currentGraph->Nodes) {
        ImNodes::BeginNode(node.ID);

        ImNodes::BeginNodeTitleBar();
        ImGui::Text("%s", node.Name.c_str());
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(node.ID << 8 | 0);
        ImGui::Text("In");
        ImNodes::EndInputAttribute();

        if (node.ID == currentGraph->EntryNodeID) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "[ENTRY]");
        }
        if (!node.AnimationPath.empty()) {
            std::string filename = std::filesystem::path(node.AnimationPath).stem().string();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Anim: %s", filename.c_str());
        }
        else {
            ImGui::TextDisabled("No Anim");
        }

        ImNodes::BeginOutputAttribute(node.ID << 8 | 1);
        ImGui::Text("Out");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    for (const auto& link : currentGraph->Transitions) {
        ImNodes::Link(link.ID, link.FromNodeID << 8 | 1, link.ToNodeID << 8 | 0);
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("NodeGraphContext");
    }

    if (ImGui::BeginPopup("NodeGraphContext")) {
        popupPos = ImGui::GetMousePosOnOpeningCurrentPopup();

        if (ImGui::MenuItem("Add State Node")) {
            GraphNode newNode;
            newNode.ID = ++currentGraph->NextNodeID;
            newNode.Name = "New State";
            newNode.AnimationPath = "";
            newNode.EditorPosition = popupPos;


            if (currentGraph->EntryNodeID == -1) currentGraph->EntryNodeID = newNode.ID;

            currentGraph->Nodes.push_back(newNode);
        }
        ImGui::EndPopup();
    }

    ImNodes::EndNodeEditor();

    int startAttr, endAttr;
    if (ImNodes::IsLinkCreated(&startAttr, &endAttr)) {
        int startNode = startAttr >> 8;
        int endNode = endAttr >> 8;

        GraphTransition newTrans;
        newTrans.ID = ++currentGraph->NextLinkID;
        newTrans.FromNodeID = startNode;
        newTrans.ToNodeID = endNode;
        newTrans.ConditionParam = ""; 
        newTrans.Operation = ConditionOp::Greater;
        newTrans.Threshold = 0.0f;

        currentGraph->Transitions.push_back(newTrans);
    }

    int numSelectedNodes = ImNodes::NumSelectedNodes();
    int numSelectedLinks = ImNodes::NumSelectedLinks();

    if (numSelectedNodes > 0)
    {
        std::vector<int> selectedIDs;
        selectedIDs.resize(numSelectedNodes);
        ImNodes::GetSelectedNodes(selectedIDs.data());

        selectedNodeID = selectedIDs[0];
        selectedLinkID = -1;
    }
    else if (numSelectedLinks > 0)
    {
        std::vector<int> selectedIDs;
        selectedIDs.resize(numSelectedLinks);
        ImNodes::GetSelectedLinks(selectedIDs.data());

        selectedLinkID = selectedIDs[0];
        selectedNodeID = -1;
    }
    else
    {
        if (ImGui::IsMouseClicked(0) && ImNodes::IsEditorHovered())
        {
            selectedNodeID = -1;
            selectedLinkID = -1;
        }
    }
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        if (selectedNodeID != -1) {
            auto& nodes = currentGraph->Nodes;
            nodes.erase(std::remove_if(nodes.begin(), nodes.end(),
                [this](const GraphNode& n) { return n.ID == selectedNodeID; }), nodes.end());

            auto& links = currentGraph->Transitions;
            links.erase(std::remove_if(links.begin(), links.end(),
                [this](const GraphTransition& t) { return t.FromNodeID == selectedNodeID || t.ToNodeID == selectedNodeID; }), links.end());

            if (currentGraph->EntryNodeID == selectedNodeID) currentGraph->EntryNodeID = -1;
            selectedNodeID = -1;
        }

        if (selectedLinkID != -1) {
            auto& links = currentGraph->Transitions;
            links.erase(std::remove_if(links.begin(), links.end(),
                [this](const GraphTransition& t) { return t.ID == selectedLinkID; }), links.end());
            selectedLinkID = -1;
        }
    }


    

    ImGui::EndChild();
    ImGui::End();
}

void AnimGraphEditor::LoadGraph(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        LOGF_ERROR("Failed to open graph: %s", path.c_str());
        return;
    }

    json j;
    file >> j;

    currentGraph = std::make_shared<AnimationGraphResource>();
    currentFilePath = path;

    if (j.contains("entryNodeId")) currentGraph->EntryNodeID = j["entryNodeId"];

    if (j.contains("parameters")) {
        for (auto& [key, val] : j["parameters"]["floats"].items())
            currentGraph->DefaultBlackboard[key] = AnimVar(val.get<float>());
        for (auto& [key, val] : j["parameters"]["bools"].items())
            currentGraph->DefaultBlackboard[key] = AnimVar(val.get<bool>());
    }

    for (auto& jNode : j["nodes"]) {
        GraphNode node;
        node.ID = jNode["id"];


        if (jNode.contains("name")) node.Name = jNode["name"];
        else if (jNode.contains("animName")) node.Name = jNode["animName"];

        if (jNode.contains("animPath")) node.AnimationPath = jNode["animPath"];

        node.EditorPosition = { jNode["x"], jNode["y"] };
        ImNodes::SetNodeEditorSpacePos(node.ID, node.EditorPosition);

        if (node.ID > currentGraph->NextNodeID) currentGraph->NextNodeID = node.ID;

        currentGraph->Nodes.push_back(node);
    }

    for (auto& jTrans : j["transitions"]) {
        GraphTransition trans;
        trans.ID = jTrans["id"];
        trans.FromNodeID = jTrans["from"];
        trans.ToNodeID = jTrans["to"];
        trans.ConditionParam = jTrans["condition"];
        trans.Threshold = jTrans["threshold"];
        trans.Operation = (ConditionOp)jTrans["op"];

        if (trans.ID > currentGraph->NextLinkID) currentGraph->NextLinkID = trans.ID;

        currentGraph->Transitions.push_back(trans);
    }

    closed = false;
    minimized = false;
}

void AnimGraphEditor::SaveGraph()
{
    if (!currentGraph || currentFilePath.empty()) return;

    json j;
    j["entryNodeId"] = currentGraph->EntryNodeID;


    j["parameters"] = { {"floats", json::object()}, {"bools", json::object()} };
    for (auto& [name, var] : currentGraph->DefaultBlackboard) {
        if (var.Type == AnimVarType::Float) j["parameters"]["floats"][name] = var.fVal;
        else j["parameters"]["bools"][name] = var.bVal;
    }


    j["nodes"] = json::array();
    for (auto& node : currentGraph->Nodes) {
        ImVec2 pos = ImNodes::GetNodeEditorSpacePos(node.ID);
        j["nodes"].push_back({
            {"id", node.ID},
            {"name", node.Name},
            {"animPath", node.AnimationPath},
            {"x", pos.x},
            {"y", pos.y}
            });
    }


    j["transitions"] = json::array();
    for (auto& trans : currentGraph->Transitions) {
        j["transitions"].push_back({
            {"id", trans.ID},
            {"from", trans.FromNodeID},
            {"to", trans.ToNodeID},
            {"condition", trans.ConditionParam},
            {"threshold", trans.Threshold},
            {"op", (int)trans.Operation}
            });
    }

    std::ofstream file(currentFilePath);
    if (file.is_open()) {
        file << j.dump(4);
        LOGF_INFO("Saved Animation Graph to %s", currentFilePath.c_str());
    }
}