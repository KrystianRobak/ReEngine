#pragma once

#include "imgui/imgui.h"
#include "imgui/imnodes.h"

struct Pin
{
    int id;
    std::string label;
    ImNodesPinShape shape;
    enum Type { Input, Output } type;
    // Potentially add a data type (e.g., "Color", "Float", "Execution")
    // int data_type;
};

struct Link
{
    int id;
    int start_pin_id;
    int end_pin_id;
};

struct BaseNode
{
    // Base properties...
    int id;
    std::string title;
    ImVec2 position;
    std::vector<Pin> Inputpins;
	std::vector<Pin> Outputpins;
    std::vector<Link> links;
	ImVec4 color = ImVec4(1.0f, 0.5f, 0.2f, 1.0f);
	ImVec4 titleColor = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);

    // Virtual function for the unique UI part
    virtual void DrawNodeContents() {}

    // Final DrawNode wrapper
    void DrawNode()
    {
        ImNodes::PushColorStyle(ImNodesCol_TitleBar, ImGui::GetColorU32(titleColor));
        ImNodes::PushColorStyle(ImNodesCol_NodeBackground, ImGui::GetColorU32(color));
        ImNodes::BeginNode(id);

        ImNodes::BeginNodeTitleBar();
        ImGui::Text("%s", title.c_str());
        ImNodes::EndNodeTitleBar();


        for (const auto& pin : Inputpins)
        {
            ImNodes::BeginInputAttribute(pin.id, pin.shape);
            ImGui::Text("%s", pin.label.c_str());
            ImNodes::EndInputAttribute();
        }

        DrawNodeContents(); 

        for (const auto& pin : Outputpins)
        {
            ImNodes::BeginOutputAttribute(pin.id, pin.shape);
            ImGui::Text("%s", pin.label.c_str());
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();

		ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
    }

    // Virtual destructor for safety
    virtual ~BaseNode() = default;
};
