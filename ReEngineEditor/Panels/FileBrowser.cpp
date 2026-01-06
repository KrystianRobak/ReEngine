#include "FileBrowser.h"
#include "AssetManagement/AssetSerializer.h"
#include "Logger.h"
#include <fstream>

#include <json/json.hpp>

using json = nlohmann::json;
// Define your specific engine extensions here


FileBrowser::FileBrowser() {
    // Load your icons once
}



void FileBrowser::OnInit()
{
    engineAPI->AddEventListener(Events::Window::FILE_DROPPED, [this](Event& e) {
        std::string droppedPath = e.GetParam<std::string>("FilePath");
        LOGF_INFO("Detected file drop, path to dropped: %s", droppedPath.c_str())
        auto AssetImportedType = AssetSerializer::ImportAndCookFile(droppedPath, this->currentPath);
        switch (AssetImportedType.first)
        {
            case AssetType::Texture:
                engineAPI->GetAssetManager()->GetTexture(AssetImportedType.second);
        }
        FindFiles(currentPath);
		});

    FindFiles(".");
}

void FileBrowser::FindFiles(const std::string& folderPath) {
    currentPath = folderPath;
    currentItems.clear();

    try {
        // 1. Gather Folders first
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_directory()) {
                if (entry.path().filename().string()[0] == '.') continue; // Skip hidden

                BrowserItem item;
                item.entry = entry;
                item.type = FileType::Folder;
                item.name = entry.path().filename().string();
                currentItems.push_back(item);
            }
        }

        // 2. Gather Files (Only specific extensions)
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (!entry.is_directory()) {
                std::string ext = entry.path().extension().string();
                FileType type = GetFileType(ext);

                // Filter: Only show Known Types (Custom Assets + Code)
                if (type != FileType::Unknown) {
                    BrowserItem item;
                    item.entry = entry;
                    item.type = type;
                    item.name = entry.path().filename().string();
                    item.extension = ext;
                    currentItems.push_back(item);
                }
            }
        }
    }
    catch (const std::exception& e) {
        // Handle error
    }
}

void FileBrowser::RenderItem(const BrowserItem& item, float itemWidth, float itemSpacing, int itemsPerRow, int& itemsInRow) {
    
    if (item.type < FileType::Folder || item.type > FileType::Animation)
    {
        return;
    }
    ImGui::BeginGroup();

    
    ImTextureID iconID = (ImTextureID)icons[item.type]->id;
    std::string payloadType = GetDragPayloadType(item.type);
    std::string fullPath = item.entry.path().string();

    // --- Drag & Drop Source ---
    // We allow dragging generic folders, but strictly typed assets
    ImGui::PushID(fullPath.c_str());

    // Render Image Button
    // We use ImageButton to make it clickable easily, or just Image + IsItemClicked
    ImGui::Image((void*)(intptr_t)iconID, ImVec2(itemWidth, itemWidth));

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        // payload: "Name|Path"
        std::string payloadData = item.name + "|" + fullPath;

        // IMPORTANT: The Payload Label depends on the File Type!
        // This lets your Component Inspector accept "ASSET_TEXTURE" but reject "ASSET_MESH"
        ImGui::SetDragDropPayload(payloadType.c_str(), payloadData.c_str(), payloadData.size() + 1);

        // Preview
        ImGui::Image((void*)(intptr_t)iconID, ImVec2(32, 32));
        ImGui::Text("%s", item.name.c_str());

        ImGui::EndDragDropSource();
    }

    // Text Label
    ImGui::TextWrapped("%s", item.name.c_str());

    ImGui::PopID();
    ImGui::EndGroup();

    // --- Interaction ---
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (item.type == FileType::Folder) {
            pathHistory.push_back(currentPath);
            FindFiles(fullPath);
        }
        else if (item.type == FileType::Material) {
            Event event(Events::Editor::MaterialSystem::OPEN_MATERIAL_FILE);
            event.SetParam<std::string>("PATH", fullPath);

            engineAPI->SendEvent(event);
        }
        else if (item.type == FileType::Scene) {
            // Load Scene
            engineAPI->OpenScene(fullPath);
        }
        else if (item.type == FileType::Animation) {
            // Trigger the Event
            Event event(Events::Editor::StateMachineGraph::OPEN_STATEMACHINE_FILE);
            event.SetParam<std::string>("PATH", fullPath);
            engineAPI->SendEvent(event);
        }
    }

    // Layout Logic
    itemsInRow++;
    if (itemsInRow >= itemsPerRow) {
        itemsInRow = 0;
        ImGui::NewLine();
    }
    else {
        ImGui::SameLine(0, itemSpacing);
    }
}

void FileBrowser::Render() {
    ImGui::Begin("Content Browser");

    // Top Bar (Back Button)
    if (ImGui::Button("<- Back") && !pathHistory.empty()) {
        std::string parent = pathHistory.back();
        pathHistory.pop_back();
        FindFiles(parent);
    }
    ImGui::SameLine();
    ImGui::Text("Path: %s", currentPath.c_str());
    ImGui::Separator();

    if (ImGui::BeginPopupContextWindow("CreateFilePopUp"))
    {
        if (ImGui::MenuItem("Create Scene"))
        {

        }
        if (ImGui::MenuItem("Create Material"))
        {
            std::string fileName = "NewMaterial.material";
            std::filesystem::path filePath = std::filesystem::path(currentPath) / fileName;

            // Ensure the file name is unique
            int counter = 1;
            while (std::filesystem::exists(filePath))
            {
                fileName = "NewMaterial_" + std::to_string(counter++) + ".material";
                filePath = std::filesystem::path(currentPath) / fileName;
            }

            json j;
            j["id"] = 1;
            j["name"] = fileName;
            j["path"] = filePath.string();
            j["nodes"] = json::array();
            j["links"] = json::array();

            // Ensure parent directory exists
            const std::filesystem::path materialPath = filePath;
            if (!materialPath.parent_path().empty())
            {
                std::filesystem::create_directories(materialPath.parent_path());
            }

            // Write JSON to file using filesystem path
            std::ofstream file(materialPath, std::ios::out | std::ios::trunc);
            if (file)
            {
                file << j.dump(4); // Pretty print
                file.close();

                std::cout << "Created material file: " << materialPath << std::endl;
            }
            else
            {
                std::cerr << "Failed to create file: " << materialPath << std::endl;
            }

            // Notify the engine/editor
            Event event(Events::Editor::MaterialSystem::CREATE_MATERIAL_FILE);
            event.SetParam<std::string>("PATH", materialPath.string());
            engineAPI->SendEvent(event);
            FindFiles(currentPath);
        }
        // --- NEW: CREATE ANIMATION GRAPH ---
        if (ImGui::MenuItem("Create Animation Graph"))
        {
            std::string fileName = "NewStateGraph.rsm"; // .rsm = ReEngine State Machine
            std::filesystem::path filePath = std::filesystem::path(currentPath) / fileName;

            // 1. Ensure Unique Filename
            int counter = 1;
            while (std::filesystem::exists(filePath))
            {
                fileName = "NewStateGraph_" + std::to_string(counter++) + ".rsm";
                filePath = std::filesystem::path(currentPath) / fileName;
            }

            // 2. Create Default JSON Structure
            json j;
            j["entryNodeId"] = -1;       // No entry state set
            j["nodes"] = json::array();  // Empty list of states
            j["transitions"] = json::array();

            // Default Blackboard parameters
            j["parameters"] = {
                {"floats", json::object()},
                {"bools", json::object()}
            };

            // 3. Write to Disk
            std::ofstream file(filePath, std::ios::out | std::ios::trunc);
            if (file)
            {
                file << j.dump(4); // Pretty print
                file.close();
                LOGF_INFO("Created Animation Graph: %s", filePath.string().c_str());
            }
            else
            {
                LOGF_ERROR("Failed to create file: %s", filePath.string().c_str());
            }

            // 4. Refresh Browser to show new file
            FindFiles(currentPath);
        }
        ImGui::EndPopup();
    }

    // Grid Layout
    float padding = 16.0f;
    float thumbnailSize = 64.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    int itemsInRow = 0;

    for (const auto& item : currentItems) {
        RenderItem(item, thumbnailSize, padding, columnCount, itemsInRow);
    }

    ImGui::End();
}