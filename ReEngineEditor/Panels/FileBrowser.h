#pragma once
#include "UIComponent.h"
#include <filesystem>
#include <map>
#include "AssetFileFormat.h"


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

	void OnInit() override;

    GLuint GetIconForType(FileType type);
    std::string GetDragPayloadType(FileType type);

    std::vector<BrowserItem> currentItems; // Unified list of folders and files

    std::string currentPath;
    std::vector<std::string> pathHistory;

    // Icons
    std::map<FileType, std::shared_ptr<TextureResource>> icons;
    GLuint LoadTexture(const std::string& path);
};