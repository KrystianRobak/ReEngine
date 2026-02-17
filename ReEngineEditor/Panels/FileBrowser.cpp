#include "FileBrowser.h"
#include "AssetManagement/AssetSerializer.h"
#include "Logger.h"
#include <fstream>
#include <iostream>

#include <json/json.hpp>

using json = nlohmann::json;

FileBrowser::FileBrowser() {

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
            break;
        }
        FindFiles(currentPath);
        });

    FindFiles(".");
}

void FileBrowser::FindFiles(const std::string& folderPath) {
    currentPath = folderPath;
    currentItems.clear();

    try {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_directory()) {
                if (entry.path().filename().string()[0] == '.') continue;

                BrowserItem item;
                item.entry = entry;
                item.type = FileType::Folder;
                item.name = entry.path().filename().string();
                currentItems.push_back(item);
            }
        }

        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (!entry.is_directory()) {
                std::string ext = entry.path().extension().string();
                FileType type = GetFileType(ext);

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
        LOGF_ERROR("Error finding files: %s", e.what());
    }
}

void FileBrowser::RenderItem(const BrowserItem& item, float itemWidth, float itemSpacing, int itemsPerRow, int& itemsInRow) {

    if (item.type < FileType::Folder || item.type > FileType::Prefab)
        return;

    ImGui::BeginGroup();

    ImTextureID iconID = (ImTextureID)icons[item.type]->id;
    std::string payloadType = GetDragPayloadType(item.type);
    std::string fullPath = item.entry.path().string();
    std::string fileName = item.name;

    ImGui::PushID(fullPath.c_str());

    ImGui::Image((void*)(intptr_t)iconID, ImVec2(itemWidth, itemWidth));

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        std::string payloadData = item.name + "|" + fullPath;
        ImGui::SetDragDropPayload(payloadType.c_str(), payloadData.c_str(), payloadData.size() + 1);
        ImGui::Image((void*)(intptr_t)iconID, ImVec2(32, 32));
        ImGui::Text("%s", item.name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginPopupContextItem("ItemContextMenu"))
    {
        if (ImGui::MenuItem("Rename"))
        {
            m_PathToRename = fullPath;
            memset(m_RenameBuf, 0, sizeof(m_RenameBuf));
            strncpy_s(m_RenameBuf, fileName.c_str(), sizeof(m_RenameBuf) - 1);
            m_RequestRenamePopup = true;
        }

        if (ImGui::MenuItem("Delete"))
        {
            try {
                std::filesystem::remove_all(fullPath);
                m_Dirty = true;
            }
            catch (const std::exception& e) {
                LOGF_ERROR("Failed to delete: %s", e.what());
            }
        }
        ImGui::EndPopup();
    }

    ImGui::TextWrapped("%s", item.name.c_str());

    ImGui::PopID();
    ImGui::EndGroup();

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
            engineAPI->OpenScene(fullPath);
        }
        else if (item.type == FileType::Animation) {
            Event event(Events::Editor::StateMachineGraph::OPEN_STATEMACHINE_FILE);
            event.SetParam<std::string>("PATH", fullPath);
            engineAPI->SendEvent(event);
        }
    }

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

    if (m_Dirty) {
        FindFiles(currentPath);
        m_Dirty = false;
    }

    if (ImGui::Button("<- Back") && !pathHistory.empty()) {
        std::string parent = pathHistory.back();
        pathHistory.pop_back();
        FindFiles(parent);
    }
    ImGui::SameLine();
    ImGui::Text("Path: %s", currentPath.c_str());
    ImGui::Separator();

    if (ImGui::BeginPopupContextWindow("CreateFilePopUp", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        if (ImGui::MenuItem("Create Folder"))
        {
            std::string folderName = "NewFolder";
            std::filesystem::path folderPath = std::filesystem::path(currentPath) / folderName;

            int counter = 1;
            while (std::filesystem::exists(folderPath)) {
                folderName = "NewFolder_" + std::to_string(counter++);
                folderPath = std::filesystem::path(currentPath) / folderName;
            }

            try {
                std::filesystem::create_directory(folderPath);
                m_Dirty = true;
            }
            catch (std::exception& e) {
                LOGF_ERROR("Could not create folder: %s", e.what());
            }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Create Scene"))
        {
            // Scene creation logic...
        }

        if (ImGui::MenuItem("Create Material"))
        {
            std::string fileName = "NewMaterial.material";
            std::filesystem::path filePath = std::filesystem::path(currentPath) / fileName;
            int counter = 1;
            while (std::filesystem::exists(filePath)) {
                fileName = "NewMaterial_" + std::to_string(counter++) + ".material";
                filePath = std::filesystem::path(currentPath) / fileName;
            }

            json j;
            j["id"] = 1;
            j["name"] = fileName;
            j["path"] = filePath.string();
            j["nodes"] = json::array();
            j["links"] = json::array();

            if (!filePath.parent_path().empty())
                std::filesystem::create_directories(filePath.parent_path());

            std::ofstream file(filePath, std::ios::out | std::ios::trunc);
            if (file) {
                file << j.dump(4);
                file.close();
            }

            Event event(Events::Editor::MaterialSystem::CREATE_MATERIAL_FILE);
            event.SetParam<std::string>("PATH", filePath.string());
            engineAPI->SendEvent(event);
            m_Dirty = true;
        }

        if (ImGui::MenuItem("Create Animation Graph"))
        {
            std::string fileName = "NewStateGraph.rsm";
            std::filesystem::path filePath = std::filesystem::path(currentPath) / fileName;
            int counter = 1;
            while (std::filesystem::exists(filePath)) {
                fileName = "NewStateGraph_" + std::to_string(counter++) + ".rsm";
                filePath = std::filesystem::path(currentPath) / fileName;
            }

            json j;
            j["entryNodeId"] = -1;
            j["nodes"] = json::array();
            j["transitions"] = json::array();
            j["parameters"] = { {"floats", json::object()}, {"bools", json::object()} };

            std::ofstream file(filePath, std::ios::out | std::ios::trunc);
            if (file) {
                file << j.dump(4);
                file.close();
            }
            m_Dirty = true;
        }

        ImGui::EndPopup();
    }

    if (m_RequestRenamePopup) {
        ImGui::OpenPopup("Rename Item");
        m_RequestRenamePopup = false;
    }

    if (ImGui::BeginPopupModal("Rename Item", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter new name:");

        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere(0);

        bool committed = false;

        if (ImGui::InputText("##rename", m_RenameBuf, sizeof(m_RenameBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
            committed = true;
        }

        if (ImGui::Button("Save") || committed)
        {
            std::filesystem::path oldPath(m_PathToRename);
            std::filesystem::path newPath = oldPath.parent_path() / m_RenameBuf;

            if (!std::string(m_RenameBuf).empty() && !std::filesystem::exists(newPath))
            {
                try {
                    std::filesystem::rename(oldPath, newPath);
                    m_Dirty = true;
                }
                catch (std::exception& e) {
                    LOGF_ERROR("Rename failed: %s", e.what());
                }
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

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