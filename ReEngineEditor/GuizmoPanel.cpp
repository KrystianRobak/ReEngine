#include "GuizmoPanel.h"


#include "ReScene.h"
#include "ReCamera.h"
#include <gtc/type_ptr.hpp>

void GuizmoPanel::OnInit()
{
    engineAPI->AddEventListener(Events::Editor::Gizmo::TRANSLATE, [&](Event& event)
        {
            currentGizmoOperation = ImGuizmo::TRANSLATE;
		});

    engineAPI->AddEventListener(Events::Editor::Gizmo::ROTATE, [&](Event& event)
        {
            currentGizmoOperation = ImGuizmo::ROTATE;
		});

    engineAPI->AddEventListener(Events::Editor::Gizmo::SCALE, [&](Event& event)
        {
            currentGizmoOperation = ImGuizmo::SCALE;
        });
}

void GuizmoPanel::Render()
{
	Entity selectedEntity = engineAPI->GetSelectedEntity();

    if (selectedEntity != 0)
    {

        ReScene* scene = engineAPI->GetCurrentScene();
        Camera* camera = scene->GetDefaultCamera();
        Transform* transform = (Transform*)engineAPI->GetComponent(engineAPI->GetSelectedEntity(), "Transform");

        ImGui::Begin("Scene");

        ImGuizmo::BeginFrame();
        ImGuizmo::SetDrawlist();

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

        glm::mat4 view = ReCamera::GetViewMatrix(*camera);
        glm::mat4 projection = ReCamera::GetProjectionMatrix(*camera);
        glm::mat4 model = ReCamera::GetModelMatrix(*transform);

        ImGuizmo::Manipulate(
            glm::value_ptr(view),
            glm::value_ptr(projection),
            currentGizmoOperation,
            mode,
            glm::value_ptr(model)
        );

        if (ImGuizmo::IsUsing())
        {
            glm::vec3 translation, rotation, scale;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model),
                glm::value_ptr(translation),
                glm::value_ptr(rotation),
                glm::value_ptr(scale));
            transform->position = translation;
            transform->rotation = rotation;
            transform->scale = scale;
        }

        ImGui::End();
    }

}