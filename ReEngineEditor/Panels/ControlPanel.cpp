#include "ControlPanel.h"
#include "GamePackager.h"
#include <windows.h>
#include <shobjidl.h>
#include "ReScene.h"

inline const char* GetMenuType(const MenuType menu) {
    switch (menu)
    {
        case MenuType::BaseMenu:
            return "Base Menu";
        case MenuType::AnimationMenu:
            return "Animation Menu";
    }

    return "Null";
}

void ControlPanel::OnInit()
{
	startIcon = GetTexture("icons/start.retex");
	pauseIcon = GetTexture("icons/stop.retex");
	recompileIcon = GetTexture("icons/recompile.retex");
}

void ControlPanel::Render()
{
    ImGui::Begin("ControlPanel");
        ImGui::BeginGroup();
        const char* items[] = {
            GetMenuType(MenuType::BaseMenu),
            GetMenuType(MenuType::AnimationMenu)
        };

        if (ImGui::ImageButton("##playButton", (isPlaying ? (void*)(intptr_t)startIcon->id : (void*)(intptr_t)pauseIcon->id), ImVec2(20, 20)))
        {
            isPlaying = !isPlaying;
            if (isPlaying)
            {
                engineApp->SetState(ApplicationState::Editor);
            }
            else
            {
                engineApp->SetState(ApplicationState::Play);
            }
        }

			
        ImGui::SameLine();

        if (ImGui::ImageButton("recompileButton",(void*)(intptr_t)recompileIcon->id, ImVec2(20, 20)))
        {
			engineAPI->SaveScene(engineAPI->GetCurrentScene()->GetPath());
        }
        ImGui::SameLine();
        if (ImGui::Button("Hot Reload DLL")) {
            engineApp->RequestRecompile();
        }
        ImGui::SameLine();
        if (ImGui::Button("Package Game")) {
            std::string path = OpenFolderDialog();
            if (!path.empty()) {
                GamePackager::PackageGame("MyAwesomeGame_Build", path);
            }
        }
        ImGui::EndGroup();
    ImGui::End();
}

void ControlPanel::OnPackageGameClicked()
{
}

std::string ControlPanel::OpenFolderDialog() {
    std::string folderPath = "";
    IFileOpenDialog* pFileOpen;

    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

    if (SUCCEEDED(hr)) {
        DWORD dwOptions;
        if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions))) {
            pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }
        hr = pFileOpen->Show(NULL);

        if (SUCCEEDED(hr)) {
            IShellItem* pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                if (SUCCEEDED(hr)) {
                    std::wstring ws(pszFilePath);
                    folderPath = std::string(ws.begin(), ws.end());
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileOpen->Release();
    }
    return folderPath;
}
