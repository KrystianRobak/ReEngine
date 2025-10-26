#include "MaterialGraphPanel.h"
#include <algorithm>

// --- Local Data Structures ---



// --------------------------------------------------------

void MaterialGraphPanel::OnInit()
{
    // Initialize ImNodes context
    ImNodes::CreateContext();

	BaseNode* exampleNode = new BaseNode();
    exampleNode->id = m_NextNodeID++;
    exampleNode->title = "Example Node";
    exampleNode->position = ImVec2(100, 100);
	exampleNode->color = ImVec4(0.4f, 0.3f, 0.7f, 1.0f);
	exampleNode->titleColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    // Define input pin
    Pin inputPin;
    inputPin.id = GetPinID(exampleNode->id, 0);
    inputPin.label = "Input";
    inputPin.shape = ImNodesPinShape_Circle;
    inputPin.type = Pin::Input;
    exampleNode->Inputpins.push_back(inputPin);
    // Define output pin
    Pin outputPin;
    outputPin.id = GetPinID(exampleNode->id, 1);
    outputPin.label = "Output";
    outputPin.shape = ImNodesPinShape_Circle;
    outputPin.type = Pin::Output;
    exampleNode->Outputpins.push_back(outputPin);
	m_Nodes.push_back(exampleNode);
}

void MaterialGraphPanel::Render()
{
    ImGui::Begin("Material Graph");

    ImNodes::BeginNodeEditor();

    for (auto& node : m_Nodes)
    {
		node->DrawNode();
    }

    ImNodes::EndNodeEditor();

    ImGui::End();
}

