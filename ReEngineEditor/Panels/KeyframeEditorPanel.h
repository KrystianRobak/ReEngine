#pragma once
#include "UIComponent.h"
#include "Engine/Core/Coordinator/Coordinator.h"
#include "Engine/Systems/Animation/AnimationSequence.h"
#include "glm/glm.hpp"

class KeyframeEditorPanel : public UIComponent {
private:
    AnimationSequencer* sequencer;
    
public:
    KeyframeEditorPanel() {
        std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
        sequencer = coordinator->GetScene();
    }

    virtual void Render() override {
        ImGui::Begin("Keyframe Editor");
        
        Keyframe* selectedKeyframe = sequencer->GetSelectedKeyframe();
        int selectedTrack = sequencer->GetSelectedTrack();
        
        if (selectedKeyframe && selectedTrack >= 0) {
            std::string Name = sequencer->sequencer->AnimPaths[selectedTrack]->Name;
            ImGui::Text("Selected Track: %s", Name.c_str());
            
            ImGui::Separator();
            ImGui::Text("Keyframe Properties");

            // Time editing
            float time = selectedKeyframe->time;
            if (ImGui::DragFloat("Time", &time, 0.01f)) {
                selectedKeyframe->time = std::clamp(time, 
                    sequencer->sequencer->timeline_start,
                    sequencer->sequencer->timeline_end);
            }

            const char* items[] = {
                "Linear Interpolation",
                "Cubic Interpolation",
                "Quartic Interpolation",
            };

            if (ImGui::Combo("Select Menu", &selectedKeyframe->interpolation, items, 2))
            {

            }
            
            // Position editing
            ImGui::Text("Position");
            ImGui::DragFloat3("Position", &selectedKeyframe->position.x, 0.1f);
            
            // Rotation editing
            ImGui::Text("Rotation");
            ImGui::DragFloat3("Rotation", &selectedKeyframe->rotation.x, 0.1f);
            
            if (ImGui::Button("Delete Keyframe")) {
                // Find and remove the keyframe from its animation
                auto& animations = sequencer->sequencer->AnimPaths[selectedTrack]->Animations;
                for (auto& anim : animations) {
                    auto it = std::find_if(anim.keyframes.begin(), anim.keyframes.end(),
                        [selectedKeyframe](const Keyframe& kf) {
                            return &kf == selectedKeyframe;
                        });
                    if (it != anim.keyframes.end()) {
                        anim.keyframes.erase(it);
                        break;
                    }
                }
                selectedKeyframe = nullptr;
            }
        } else {
            ImGui::Text("No keyframe selected");
        }
        
        ImGui::End();
    }
};