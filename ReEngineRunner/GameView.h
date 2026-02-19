#include "UIComponent.h"
#include "ILayer.h"
#include "IViewport.h"
#include "ReScene.h"

class GameView : public UIComponent
{
    virtual void OnInit() override
    {

        viewport = engineApp->CreateNewViewport("SceneViewport", 1920, 1080);

        if (engineAPI->GetCurrentScene()) {
            viewport->SetCamera(engineAPI->GetCurrentScene()->GetDefaultCamera());
        }

        engineAPI->AddEventListener(Events::Application::CAMERA_CHANGED, [this](Event& e) {
            if (engineAPI->GetCurrentScene())
                this->viewport->SetCamera(engineAPI->GetCurrentScene()->GetActiveCamera());
            });
    }

    virtual void Render() override
    {
        // 1. Get the main OS window viewport (The "Work" area)
        ImGuiViewport* main_viewport = ImGui::GetMainViewport();

        // 2. Force this window to fill the entire OS window
        ImGui::SetNextWindowPos(main_viewport->WorkPos);
        ImGui::SetNextWindowSize(main_viewport->WorkSize);
        ImGui::SetNextWindowViewport(main_viewport->ID);

        // 3. Style: Remove rounding, borders, and padding so the image fits perfectly
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        // 4. Flags:
        // NoDecoration: No title bar, resize handles, or scrollbars
        // NoMove/NoResize: Locks it in place
        // NoBringToFrontOnFocus: CRITICAL. Prevents this window from jumping over your UI when you click the game.
        // NoNavFocus: Prevents controller/keyboard navigation from selecting this window background.
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoDocking;

        // 5. Begin the "Background" Window
        // Important: We call this "GameRenderView"
        if (ImGui::Begin("GameRenderView", nullptr, window_flags))
        {
            // Get exact size for the render texture to avoid stretching/aspect ratio issues
            ImVec2 windowSize = ImGui::GetContentRegionAvail();

            // Draw the texture filling the window
            if (viewport && viewport->GetTexture()) {
                ImGui::Image(reinterpret_cast<void*>(viewport->GetTexture()), windowSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
            }
        }
        ImGui::End();
        ImGui::PopStyleVar(3); // Pop the 3 style vars we pushed
    }
private:
    IViewport* viewport = nullptr;
    ImVec2 size{ 1920, 1080 };
};

class GameLayer : public ILayer
{
public:
    virtual void OnAttach() override {
        AddUIComponent(std::make_unique<GameView>());
        ILayer::OnAttach();
       
    }

    virtual void OnEvent(Event& event) override {}
    virtual const char* GetName() const override { return "GameLayer"; }
};