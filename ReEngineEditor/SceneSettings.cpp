#include "SceneSettings.h"



void SceneSettings::OnInit()
{

}

void SceneSettings::Render()
{
	ImGui::Begin("Scene Settings");
	auto scene = engineAPI->GetCurrentScene();
	if (scene)
	{
		camera = scene->GetActiveCamera();
		if (camera)
		{
			if (ImGui::CollapsingHeader("Camera Settings"))
			{
				ImGui::DragFloat3("Position", &camera->CameraTransform.position.x, 0.1f);
				ImGui::DragFloat3("Rotation", &camera->CameraTransform.rotation.x, 0.1f);
				ImGui::DragFloat("FOV", &camera->fov, 1.0f, 1.0f, 120.0f);
				ImGui::DragFloat("Aspect Ratio", &camera->aspectRatio, 0.01f, 0.1f, 4.0f);
			}
		}
		else
		{
			ImGui::Text("No camera found in the scene.");
		}

	}
	else
	{
		ImGui::Text("No scene loaded.");
	}
	ImGui::End();
}
