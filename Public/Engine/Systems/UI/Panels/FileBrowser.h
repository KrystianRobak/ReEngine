#pragma once

#include "stb_image.h"

#include "UIComponent.h"
#include <filesystem>


class FileBrowser : public UIComponent
{
public:

    FileBrowser() {
        FindFiles("..", false);
        std::string currentPath = "..";
        std::string iconPath = "icons/document.png";
        documentTextureID = LoadFileTexture(iconPath);

        iconPath = "icons/folder.png";
        folderTextureID = LoadFileTexture(iconPath);
    };
    void FindFiles(std::string folderPath, bool direction);
    void RenderFile(GLuint textureID, float itemWidth, float itemSpacing, int itemsPerRow, int& itemsInRow, std::filesystem::directory_entry& entry);
    virtual void Render();

    GLuint LoadFileTexture(const std::string& filepath);

    

private:
    std::vector<std::filesystem::directory_entry> files;
    std::vector<std::filesystem::directory_entry> directories;
    std::string currentPath;
    std::vector<std::string> lastPath;
    std::filesystem::directory_entry selectedFile;
    bool isDragging = false;
    GLuint documentTextureID;
    GLuint folderTextureID;
};

