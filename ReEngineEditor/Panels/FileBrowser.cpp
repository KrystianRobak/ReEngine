#include "FileBrowser.h"
#include "AssetManagement/AssetSerializer.h"
#include "Logger.h"
// Define your specific engine extensions here


FileBrowser::FileBrowser() {
    // Load your icons once
}



void FileBrowser::OnInit()
{
    engineAPI->AddEventListener(Events::Window::FILE_DROPPED, [this](Event& e) {
        std::string droppedPath = e.GetParam<std::string>("FilePath");
        LOGF_INFO("Detected file drop, path to dropped: %s", droppedPath.c_str())
            AssetSerializer::ImportAndCookFile(droppedPath, this->pathHistory.back());
		});

    icons[FileType::Folder] = GetTexture("icons/folder.png");
    icons[FileType::Unknown] = GetTexture("icons/file.png");
    icons[FileType::Code] = GetTexture("icons/code.png");
    icons[FileType::StaticMesh] = GetTexture("icons/mesh.png");
    icons[FileType::SkeletalMesh] = GetTexture("icons/skeleton.png");
    icons[FileType::Texture] = GetTexture("icons/texture.png");
    icons[FileType::Material] = GetTexture("icons/material.png");
    icons[FileType::Scene] = GetTexture("icons/scene.png");

    FindFiles(".");
}

std::string FileBrowser::GetDragPayloadType(FileType type) {
    switch (type) {
    case FileType::StaticMesh:   return "ASSET_STATIC_MESH";
    case FileType::SkeletalMesh: return "ASSET_SKELETAL_MESH";
    case FileType::Texture:      return "ASSET_TEXTURE";
    case FileType::Material:     return "ASSET_MATERIAL";
    case FileType::Scene:        return "ASSET_SCENE";
    case FileType::Code:         return "ASSET_CODE";
    default:                     return "ASSET_UNKNOWN";
    }
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
            // Open Material Editor
        }
        else if (item.type == FileType::Scene) {
            // Load Scene
            engineAPI->OpenScene(fullPath);
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