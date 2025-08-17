//
// Created by ragberr on 21.01.2025.
//

#include "AnimationPanel.h"

#include "imgui/imgui_internal.h"
#include "Engine/Core/Coordinator/Coordinator.h"
#include "Engine/Systems/Animation/AnimationSystem.h"

void PopulateAnimationPaths(ReSequencer* sequencer, std::set<Entity> Entities) {
    //sequencer->AnimPaths.clear();
    //std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
    //for (auto it = Entities.begin(); it != Entities.end(); it++) {
    //    AnimationPath* animated = &coordinator->GetComponent<Animated>(*it).AnimPath;
    //    sequencer->AnimPaths.emplace_back(animated);
    //}
};

void AnimationPanel::Render() {
    /*if (IsRunning) {
        value+=1;
    }
    ImGui::Begin("Animation Panel");
    ImGui::BeginGroup();

    std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
    AnimationSequencer* AnimSequencer = coordinator->GetScene();

    PopulateAnimationPaths(AnimSequencer->sequencer, coordinator->GetSystem<AnimationSystem>()->GetEntities());

    AnimSequencer->Draw();

    ImGui::EndGroup();

    ImGui::End();*/
}
