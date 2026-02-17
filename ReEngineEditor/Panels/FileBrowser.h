#pragma once
#include "UIComponent.h"
#include <filesystem>
#include <map>
#include <vector>
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

    std::vector<BrowserItem> currentItems;

    std::string currentPath;
    std::vector<std::string> pathHistory;

    std::string m_PathToRename;
    char m_RenameBuf[256] = "";
    bool m_RequestRenamePopup = false;
    bool m_Dirty = false;
};