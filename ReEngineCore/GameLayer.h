#pragma once
#include "ILayer.h"
#include <type_traits>

class GameUILayer : public ILayer
{
public:
    virtual void OnInit() override { name = "GameUILayer"; }
    virtual const char* GetName() const override { return "GameUILayer"; }
    virtual void OnEvent(Event& event) override {} // UI usually handles events via System, not Layer

    // --- Helper: Create, Init, and Open a UI Component ---
    template<typename T>
    void OpenUI()
    {
        // 1. Check if it already exists to prevent duplicates (Optional)
        for (auto& ui : uiComponents) {
            if (dynamic_cast<T*>(ui.get())) return;
        }

        // 2. Create and Init
        auto ui = std::make_unique<T>();
        if (EngineApi_ && EngineApp_) {
            ui->Init(EngineApi_, EngineApp_);
        }

        // 3. Add to list
        AddUIComponent(std::move(ui));
    }

    // --- Helper: Find and Close a UI Component ---
    template<typename T>
    void CloseUI()
    {
        for (auto& ui : uiComponents)
        {
            // If the type matches, mark it for deletion
            if (dynamic_cast<T*>(ui.get()))
            {
                // Your ILayer::OnUpdate will clean this up next frame
                ui->pendingRemove = true;
            }
        }
    }

    // Helper to close everything (e.g., going back to Main Menu)
    void CloseAll()
    {
        for (auto& ui : uiComponents) {
            ui->pendingRemove = true;
        }
    }
};