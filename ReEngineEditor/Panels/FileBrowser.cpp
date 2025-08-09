#include "FileBrowser.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

static std::string getLastElementAfterSplit(const std::string str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;

    while (std::getline(iss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    if (tokens.empty()) {
        return "";
    }
    else {
        return tokens.back();
    }
}


void FileBrowser::FindFiles(std::string folderPath, bool direction)
{
    if (direction) 
    {
        lastPath.push_back(currentPath);
    }

    currentPath = folderPath;
    files.clear();
    directories.clear();
    try {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_directory()) {
                if (!(entry.path().filename().string()[0] == '.')) {
                    directories.push_back(entry);
                }
            }
            else if(!entry.is_directory()) {
                auto extension = entry.path().extension();
                if (extension == ".jpg" || extension == ".png" || extension == ".obj" || extension == ".mtl" || extension == ".fs" || extension == ".vs" || extension == ".cpp") {
                    files.push_back(entry);
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing folder: " << e.what() << '\n';
    }
}

void FileBrowser::RenderFile(GLuint textureID, float itemWidth, float itemSpacing, int itemsPerRow, int& itemsInRow, std::filesystem::directory_entry& entry)
{
    std::string name = entry.path().filename().string();

    ImGui::BeginGroup();

    bool isDragDropActive = ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID);
    if (isDragDropActive)
    {
        std::string nameWithPath = name + "|" + entry.path().string();
        ImGui::SetDragDropPayload("DIRECTORY_ENTRY", nameWithPath.c_str(), nameWithPath.size() + 1);

        ImGui::Image((void*)(intptr_t)textureID, ImVec2(itemWidth, itemWidth));
        ImGui::TextWrapped("%s", name.c_str());

        ImGui::EndDragDropSource();
    }
    else
    {
        ImGui::Image((void*)(intptr_t)textureID, ImVec2(itemWidth, itemWidth));
        ImGui::TextWrapped("%s", name.c_str());
    }

    ImGui::EndGroup();

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        selectedFile = entry;
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            std::string path = entry.path().string();
            if (entry.is_directory()) {
                FindFiles(path, true);
            }
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

void FileBrowser::Render()
{
    std::vector<std::filesystem::directory_entry> localFiles = files;
    std::vector<std::filesystem::directory_entry> localDirectories = directories;
    
    ImGui::Begin("File Browser");

        float itemWidth = 50.0f; 
        float itemSpacing = 50.0f;

        int itemsPerRow = (ImGui::GetContentRegionAvail().x + itemSpacing) / (itemWidth + itemSpacing);

        ImGui::BeginGroup();

        ImGui::BeginGroup();
        float iconPosX = ImGui::GetCursorPosX();

        ImGui::Image((void*)(intptr_t)folderTextureID, ImVec2(itemWidth, itemWidth));
        float textPosX = iconPosX + (itemWidth - ImGui::CalcTextSize("back").x) / 2;
        ImGui::SetCursorPosX(textPosX);
        ImGui::TextWrapped("%s", "back");
        ImGui::EndGroup();
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                std::string path = lastPath.back();
                lastPath.pop_back();
                FindFiles(path, false);
            }
        }

        ImGui::SameLine();
        int itemsInRow = 1;

            for (int i = 0; i < localDirectories.size(); i++) {
                RenderFile(folderTextureID, itemWidth, itemSpacing, itemsPerRow, itemsInRow, localDirectories[i]);
            }
            for (int i = 0; i < localFiles.size(); i++) {
                RenderFile(documentTextureID, itemWidth, itemSpacing, itemsPerRow, itemsInRow, localFiles[i]);
            }
        ImGui::EndGroup();
    ImGui::End();
}

GLuint FileBrowser::LoadFileTexture(const std::string& filepath) {
    int width, height, channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (channels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else if (channels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cerr << "Unsupported number of channels: " << channels << std::endl;
        stbi_image_free(data);
        return 0;
    }

    stbi_image_free(data);

    return textureID;
}
