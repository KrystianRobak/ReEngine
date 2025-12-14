#pragma once
#include "UIComponent.h"
#include <filesystem>
#include <map>

// Define distinct asset types for Drag & Drop and Icons
enum class FileType {
    Folder,
    Unknown,
    Code,           // .cpp, .h
    StaticMesh,     // .remesh
    SkeletalMesh,   // .reskel
    Texture,        // .retex
    Material,       // .material
    Scene           // .scene (or .json)
};

struct BrowserItem {
    std::filesystem::directory_entry entry;
    FileType type;
    std::string name;
    std::string extension;
};

class FileBrowser : public UIComponent
{
public:
    FileBrowser();

    virtual void Render() override;

private:
    void FindFiles(const std::string& folderPath);
    void RenderItem(const BrowserItem& item, float itemWidth, float itemSpacing, int itemsPerRow, int& itemsInRow);

    // Helpers
    FileType GetFileType(const std::string& extension);
    GLuint GetIconForType(FileType type);
    std::string GetDragPayloadType(FileType type);

    std::vector<BrowserItem> currentItems; // Unified list of folders and files

    std::string currentPath;
    std::vector<std::string> pathHistory;

    // Icons
    std::map<FileType, GLuint> icons;
    GLuint LoadTexture(const std::string& path);
};